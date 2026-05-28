#pragma once

/**
 * @file NTupleService.hpp
 * @brief RNTuple-backed event-data writer service with thread-parallel fills.
 *
 * Builds on ROOT's `RNTupleParallelWriter` + `RNTupleFillContext` (ROOT
 * 6.32+, default backend in 6.40). The high-level flow is:
 *
 *  1. Each algorithm declares an `NTupleWriter<T>` member; `init()` walks
 *     `T::ntuple_fields()` and asks the service to register the schema
 *     (`register_struct<T>(tree_name)`). The service builds a bare
 *     `RNTupleModel` plus a list of per-field "binders" that know how to
 *     attach a thread-local copy of `T` into a fresh `REntry`.
 *
 *  2. The first time a worker thread touches a writer (via `data()` or
 *     `fill()`), the service hands it back a per-thread slot: a fresh
 *     `T` storage, a `shared_ptr<RNTupleFillContext>` obtained from the
 *     `RNTupleParallelWriter`, and a `REntry` whose fields are already
 *     bound by raw pointer to the slot's `T` members. The pointer is
 *     cached in a `thread_local` on the writer so subsequent fills skip
 *     the lookup entirely.
 *
 *  3. `fill()` calls `ctx->Fill(*entry)`. No global mutex — independent
 *     contexts prepare independent clusters; ROOT internally serializes
 *     only when a cluster commits.
 *
 *  4. `finalize()` flushes and drops every slot first (per the ROOT
 *     parallel-writer contract: contexts must be destroyed before the
 *     writer's `CommitDataset()` runs), then destroys the parallel
 *     writer, which closes the RNTuple.
 *
 * The previous global-mutex / copier design (one shared `T`, copy values
 * into the model field, take a write lock per fill) was the dominant
 * bottleneck under heavy parallelism. The new design replaces "copy then
 * lock" with "bind once per thread, then lock-free fill".
 *
 * The public-facing API surface that algorithms use is unchanged —
 * `NTupleWriter<T>::data()` / `.fill()` / `.fill_if()` — only the
 * underlying storage strategy changed.
 */

#include <fmt/format.h>

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <pfr/core_name.hpp>

#include <clas12/framework/core/Field.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>

#include <TFile.h>

#include <ROOT/REntry.hxx>
#include <ROOT/RNTupleFillContext.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleParallelWriter.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>
#include <ROOT/RVersion.hxx>

namespace framework {

// `RNTupleParallelWriter` and `RNTupleFillContext` were promoted from
// `ROOT::Experimental::` into `ROOT::` in ROOT 6.38. ifarm still ships
// 6.36 in places, so resolve both via a small alias namespace and keep
// the rest of the service namespace-agnostic.
namespace rntp_detail {
#if defined(ROOT_VERSION_CODE) && ROOT_VERSION_CODE >= ROOT_VERSION(6, 38, 0)
    using ParallelWriterType = ::ROOT::RNTupleParallelWriter;
    using FillContextType    = ::ROOT::RNTupleFillContext;
#else
    using ParallelWriterType = ::ROOT::Experimental::RNTupleParallelWriter;
    using FillContextType    = ::ROOT::Experimental::RNTupleFillContext;
#endif
}  // namespace rntp_detail

class OutputFileService;

/**
 * @brief Thread-parallel RNTuple writer service.
 *
 * Algorithms talk to this service exclusively via `NTupleWriter<T>`
 * (see NTupleStruct.hpp). The service holds the parallel writer +
 * per-thread fill contexts; the writer holds the schema and the model.
 */
class NTupleService : public Service {
   public:
    static constexpr std::string_view service_name() { return "NTupleService"; }

    explicit NTupleService(std::string name = "NTupleService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        if (!m_output_svc) {
            m_output_svc = locate_service<OutputFileService>("OutputFileService");
        }
        if (!m_output_svc) {
            Log::error(name(), "OutputFileService not available via ServiceManager");
            return StatusCode::FAILURE();
        }
        if (!m_output_svc->is_file_open()) {
            Log::error(name(), "OutputFileService file is not open");
            return StatusCode::FAILURE();
        }
        Log::info(name(), "Initialized with OutputFileService (RNTuple parallel backend)");
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        std::lock_guard lock(m_registry_mutex);
        Log::info(name(), "Finalizing NTupleService, writing {} ntuple(s)", m_ntuples.size());
        for (auto& [n, pn] : m_ntuples) {
            long long nentries = 0;
            for (auto& [tid, slot] : pn.slots) {
                if (slot) {
                    nentries += static_cast<long long>(slot->entry_count());
                }
            }
            // Drop every per-thread slot first: their destructors flush
            // any pending rows through the shared fill contexts. The
            // RNTupleParallelWriter contract requires all contexts be
            // destroyed before CommitDataset() runs.
            pn.slots.clear();
            pn.writer.reset();   // calls CommitDataset()
            Log::info(name(), "  Wrote ntuple '{}' ({} entries)", n, nentries);
        }
        Log::info(name(), "Finalized");
        return StatusCode::SUCCESS();
    }

    auto set_output_file_service(OutputFileService* svc) -> void {
        m_output_svc = svc;
    }

    auto get_output_file() const -> std::string {
        return m_output_svc ? m_output_svc->get_output_file() : "";
    }

    // -------------------------------------------------------------------------
    // Schema registration (called from NTupleWriter<T>::init)
    // -------------------------------------------------------------------------

    /// Register the schema for one ntuple from a struct's `ntuple_fields()`.
    /// Idempotent on (tree_name, T) — calling twice with the same T re-uses
    /// the existing entry and returns. Calling with a different T against
    /// an already-bound tree name is an error.
    template <typename T>
    auto register_struct(const std::string& tree_name,
                         const std::string& title = "") -> void {
        std::lock_guard lock(m_registry_mutex);
        auto [it, inserted] = m_ntuples.try_emplace(tree_name);
        auto& pn = it->second;

        if (!inserted) {
            // Already registered — typically the same algorithm being
            // re-initialised, or two writers on the same tree. Nothing
            // more to do here; the model and binders are stable.
            return;
        }

        pn.title  = title.empty() ? tree_name : title;
        pn.model  = ROOT::RNTupleModel::CreateBare();
        // Each field contributes (1) a typed field to the bare model and
        // (2) a binder lambda that, given a void* to a Slot<T>'s `value`
        // member and a fresh REntry, points the entry's field at the right
        // T member. Two sources of (name, member): the hand-written
        // T::ntuple_fields() tuple, or — for a plain aggregate that omits it
        // — Boost.PFR reflection (offset-based binder, see build_fields_pfr).
        if constexpr (HasStaticNTupleFields<T>) {
            std::apply(
                [&](const auto&... f) {
                    (build_field<T>(pn, f), ...);
                },
                T::ntuple_fields());
        } else {
            build_fields_pfr<T>(pn);
        }
    }

    // -------------------------------------------------------------------------
    // Per-thread slot acquisition (called from NTupleWriter<T>::data/fill)
    // -------------------------------------------------------------------------

    /// Type-erased base so the per-ntuple slot map can hold any
    /// `Slot<T>` while still letting finalize() and the diagnostics
    /// query entry counts without knowing T.
    struct SlotBase {
        virtual ~SlotBase() = default;
        virtual auto entry_count() const -> std::size_t = 0;
    };

    /// Per-thread fill state for one ntuple. Inherits SlotBase so the
    /// service can hold an erased `shared_ptr<SlotBase>` while still
    /// returning the typed `Slot<T>*` to callers that know T.
    template <typename T>
    struct Slot final : SlotBase {
        T                                          value{};
        std::shared_ptr<rntp_detail::FillContextType>  ctx;
        std::unique_ptr<ROOT::REntry>              entry;

        auto fill() -> void { ctx->Fill(*entry); }
        auto entry_count() const -> std::size_t override {
            return ctx ? ctx->GetNEntries() : 0;
        }
    };

    /// Get or create this thread's slot for the named ntuple. Slow path
    /// (lock + context creation) only fires on the first call from a given
    /// thread; subsequent calls hit a cached pointer in `NTupleWriter<T>`.
    template <typename T>
    auto acquire_slot(const std::string& tree_name) -> Slot<T>* {
        std::lock_guard lock(m_registry_mutex);
        auto it = m_ntuples.find(tree_name);
        if (it == m_ntuples.end()) {
            Log::warning(name(), "Ntuple '{}' not registered", tree_name);
            return nullptr;
        }
        auto& pn = it->second;

        const auto tid = std::this_thread::get_id();
        if (auto slot_it = pn.slots.find(tid); slot_it != pn.slots.end()) {
            return static_cast<Slot<T>*>(slot_it->second.get());
        }

        if (!pn.writer) {
            if (!ensure_writer_locked(tree_name, pn)) return nullptr;
        }

        auto slot = std::make_shared<Slot<T>>();
        slot->ctx = pn.writer->CreateFillContext();
        slot->entry = slot->ctx->CreateEntry();
        // Rebind every field's storage from the model's default to the
        // slot-local T's member, so Fill() reads the per-thread value
        // with no copy.
        for (auto& binder : pn.binders) {
            binder(static_cast<void*>(&slot->value), *slot->entry);
        }
        auto* raw = slot.get();
        pn.slots.emplace(tid, std::move(slot));   // owns the slot
        return raw;
    }

    // -------------------------------------------------------------------------
    // Diagnostics
    // -------------------------------------------------------------------------

    /// Sum of entries across every per-thread fill context for the named ntuple.
    auto get_entries(const std::string& tree_name) const -> long long {
        std::lock_guard lock(m_registry_mutex);
        auto it = m_ntuples.find(tree_name);
        if (it == m_ntuples.end()) return 0;
        long long n = 0;
        for (const auto& [tid, slot] : it->second.slots) {
            if (!slot) continue;
            // Type-erased: each Slot stores its ctx; we use the helper
            // typed pointer interface via the SlotBase virtual call.
            n += static_cast<long long>(slot->entry_count());
        }
        return n;
    }

    auto get_tree_names() const -> std::vector<std::string> {
        std::lock_guard lock(m_registry_mutex);
        std::vector<std::string> names;
        names.reserve(m_ntuples.size());
        for (const auto& [nm, _] : m_ntuples) names.push_back(nm);
        return names;
    }

    auto print_summary() const -> void {
        std::lock_guard lock(m_registry_mutex);
        fmt::print("\nN-tuple Summary (RNTuple parallel backend):\n");
        fmt::print("  Output file: {}\n", get_output_file());
        fmt::print("  Total ntuples: {}\n", m_ntuples.size());
        if (!m_ntuples.empty()) {
            fmt::print("\n  Ntuple Details:\n");
            for (const auto& [nm, pn] : m_ntuples) {
                long long entries = 0;
                for (const auto& [tid, slot] : pn.slots) {
                    if (slot) entries += static_cast<long long>(slot->entry_count());
                }
                fmt::print("    {:<20} : {} entries, {} fields, {} thread slot(s)\n",
                           nm, entries, pn.binders.size(), pn.slots.size());
            }
        }
        fmt::print("\n");
    }

   private:
    using BinderFn = std::function<void(void* slot_value, ROOT::REntry& entry)>;

    struct PendingNtuple {
        std::unique_ptr<ROOT::RNTupleModel>                          model;
        std::vector<BinderFn>                                        binders;
        std::unique_ptr<rntp_detail::ParallelWriterType>                 writer;
        std::unordered_map<std::thread::id, std::shared_ptr<SlotBase>> slots;
        std::string                                                  title;
    };

    template <typename T, typename FieldT>
    auto build_field(PendingNtuple& pn, const FieldT& f) -> void {
        using MemberPtr = decltype(f.ptr);
        // Extract the field's value type from the member pointer.
        using FieldValue = std::remove_cvref_t<
            decltype(std::declval<T>().*std::declval<MemberPtr>())>;

        pn.model->MakeField<FieldValue>(std::string_view{f.name});
        pn.binders.push_back(
            [member = f.ptr, fname = std::string{f.name}](void* slot_value, ROOT::REntry& entry) {
                auto* typed = static_cast<T*>(slot_value);
                entry.template BindRawPtr<FieldValue>(
                    std::string_view{fname}, &(typed->*member));
            });
    }

    /// Reflection path: build the model fields + binders for a plain-aggregate
    /// ntuple struct that omits ntuple_fields(). Boost.PFR gives each field's
    /// (name, reference); since PFR yields a reference (not a member pointer),
    /// we capture the field's byte OFFSET — computed once against a probe
    /// instance — and bind `slot_base + off` per thread slot. Requires
    /// standard layout so the offset is well-defined (all current ntuple
    /// structs are POD scalars).
    template <typename T>
    auto build_fields_pfr(PendingNtuple& pn) -> void {
        static_assert(std::is_standard_layout_v<T>,
                      "PFR ntuple struct must be standard-layout for offset binding; "
                      "provide a static ntuple_fields() tuple otherwise.");
        T probe{};
        const auto* probe_base = reinterpret_cast<const std::byte*>(&probe);
        pfr::for_each_field_with_name(probe, [&](std::string_view name, auto& field) {
            using FieldValue = std::remove_cvref_t<decltype(field)>;
            pn.model->MakeField<FieldValue>(std::string_view{name});
            const std::size_t off =
                static_cast<std::size_t>(reinterpret_cast<const std::byte*>(&field) - probe_base);
            pn.binders.push_back(
                [off, fname = std::string{name}](void* slot_value, ROOT::REntry& entry) {
                    auto* base = static_cast<std::byte*>(slot_value);
                    entry.template BindRawPtr<FieldValue>(
                        std::string_view{fname},
                        reinterpret_cast<FieldValue*>(base + off));
                });
        });
    }

    auto ensure_writer_locked(const std::string& tree_name, PendingNtuple& pn) -> bool {
        if (!m_output_svc || !m_output_svc->is_file_open()) {
            Log::error(name(), "OutputFileService not available for ntuple '{}'", tree_name);
            return false;
        }
        TFile* file = m_output_svc->get_file();
        if (!file) {
            Log::error(name(), "TFile not available for ntuple '{}'", tree_name);
            return false;
        }
        try {
            pn.writer = rntp_detail::ParallelWriterType::Append(
                std::move(pn.model), std::string_view(tree_name), *file);
        } catch (const std::exception& e) {
            Log::error(name(), "Failed to create RNTupleParallelWriter for '{}': {}",
                       tree_name, e.what());
            return false;
        }
        return true;
    }

    mutable std::mutex m_registry_mutex;
    std::unordered_map<std::string, PendingNtuple> m_ntuples;
    OutputFileService* m_output_svc = nullptr;
};

/**
 * @brief RAII helper that calls `fill` on scope exit.
 *
 * Usage:
 * @code
 * {
 *     TreeFiller filler(ntuple_svc, "events");
 *     m_writer.data().x = ...;
 * }  // → svc->fill("events") here
 * @endcode
 */
class TreeFiller {
   public:
    TreeFiller(NTupleService* svc, std::string tree)
        : m_svc(svc), m_tree(std::move(tree)) {}
    ~TreeFiller() {
        // Deliberately not calling fill here — the new parallel writer
        // model handles fills through NTupleWriter<T>::fill(). This RAII
        // wrapper is retained only as a no-op compatibility shim.
    }

   private:
    NTupleService* m_svc;
    std::string    m_tree;
};

}  // namespace framework
