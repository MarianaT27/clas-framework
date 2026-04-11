#pragma once

/**
 * @file NTupleService.hpp
 * @brief RNTuple-backed event-data writer service.
 *
 * Replaces the previous TTree/TBranch implementation with ROOT's
 * RNTuple format (production-ready in ROOT 6.38+, the default from
 * 6.40). RNTuple gives us 20-35% smaller files, asynchronous I/O,
 * better multi-thread scaling, and removes the per-fill mutex that
 * the TTree path needed.
 *
 * The public API (register_branch / fill / get_entries / ...) is
 * source-compatible with the old service so existing analyses keep
 * compiling, but the semantics are slightly different:
 *
 *  - RNTuple requires the model (schema) to be frozen before the
 *    writer is created. `register_branch` therefore accumulates
 *    field bindings into a pending model, and the very first
 *    `fill()` call for a given ntuple seals the model, spawns the
 *    writer, and then flushes the entry. Subsequent fills go
 *    straight through to the writer.
 *
 *  - Registrations must happen before the first fill for that
 *    ntuple - the existing algorithm lifecycle (on_initialize
 *    registers, process fills) already matches this.
 *
 *  - The "leaf list" string used by TTree::Branch is accepted but
 *    ignored; RNTuple derives types from C++ template parameters.
 *
 *  - Directories inside the ROOT file are ignored. Each RNTuple is
 *    written at the file top level under its registered name.
 */

#include <fmt/format.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>

#include <TFile.h>

#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>

namespace clas12::framework {

class OutputFileService;

/**
 * @brief Service that owns RNTupleWriter instances and copies
 *        user-held field values into them on every fill.
 *
 * Usage from an algorithm:
 * @code
 * NTupleService* svc = ...;  // via ServiceManager
 * double e_p, e_theta;
 * svc->register_branch("events", "e_p",     &e_p);
 * svc->register_branch("events", "e_theta", &e_theta);
 * // ...
 * e_p     = electron.p();
 * e_theta = electron.theta();
 * svc->fill("events");
 * @endcode
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
        Log::info(name(), "Initialized with OutputFileService (RNTuple backend)");
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        std::lock_guard lock(m_mutex);
        Log::info(name(), "Finalizing NTupleService, writing {} ntuple(s)", m_ntuples.size());
        // Destroy every writer: that flushes any pending cluster and
        // commits the RNTuple to the attached TDirectory.
        for (auto& [n, pn] : m_ntuples) {
            const auto nentries = pn.writer ? pn.writer->GetNEntries() : 0;
            pn.writer.reset();
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
    // Registration
    // -------------------------------------------------------------------------

    /**
     * @brief Create a fresh ntuple slot with the given name.
     *
     * Idempotent - if the ntuple already exists this is a no-op. The
     * return value is kept for API compatibility with the old TTree
     * service (callers generally ignored it). It is guaranteed
     * non-null on success.
     */
    auto create_tree(const std::string& name,
                     const std::string& title = "",
                     const std::string& /*directory*/ = "") -> void* {
        std::lock_guard lock(m_mutex);
        auto& pn = ensure_ntuple_locked(name, title);
        return &pn;
    }

    /**
     * @brief Register a scalar field bound to a user-provided address.
     *
     * At fill time the service copies `*address` into the RNTuple
     * entry, so the analysis keeps the same "update the variable,
     * then fill()" flow the old TTree API used. The optional leaf
     * string is accepted but ignored - RNTuple infers the field
     * type from T.
     */
    template <typename T>
    auto register_branch(const std::string& tree_name,
                         const std::string& branch_name,
                         T* address,
                         const std::string& /*leaf_list*/ = "") -> void {
        if (!address) return;
        std::lock_guard lock(m_mutex);
        auto& pn = ensure_ntuple_locked(tree_name, tree_name);
        if (pn.writer) {
            Log::error(name(), "Cannot register '{}' on ntuple '{}' after first fill", branch_name, tree_name);
            return;
        }
        if (!pn.model) {
            Log::error(name(), "Ntuple '{}' has no pending model", tree_name);
            return;
        }
        auto field_ptr = pn.model->MakeField<T>(std::string_view(branch_name));
        pn.copiers.push_back([user = address, field = field_ptr]() {
            *field = *user;
        });
    }

    /**
     * @brief Register a std::vector field bound to a user-provided address.
     *
     * RNTuple represents `std::vector<T>` natively - no on-disk
     * indirection, variable-length columns are handled directly.
     */
    template <typename T>
    auto register_vector_branch(const std::string& tree_name,
                                const std::string& branch_name,
                                std::vector<T>* address) -> void {
        register_branch(tree_name, branch_name, address);
    }

    // -------------------------------------------------------------------------
    // Filling
    // -------------------------------------------------------------------------

    /**
     * @brief Fill one entry for the named ntuple.
     *
     * On the first call the pending model is frozen and an
     * RNTupleWriter is created against the OutputFileService TFile.
     * Subsequent calls copy the user-held values into the entry and
     * delegate to the writer.
     */
    auto fill(const std::string& tree_name) -> void {
        std::lock_guard lock(m_mutex);
        auto it = m_ntuples.find(tree_name);
        if (it == m_ntuples.end()) {
            Log::warning(name(), "Ntuple '{}' not found", tree_name);
            return;
        }
        auto& pn = it->second;

        if (!pn.writer) {
            if (!ensure_writer_locked(tree_name, pn)) return;
        }

        for (auto& copier : pn.copiers) copier();
        pn.writer->Fill();
    }

    /**
     * @brief Number of entries written to the named ntuple.
     */
    auto get_entries(const std::string& tree_name) const -> long long {
        std::lock_guard lock(m_mutex);
        auto it = m_ntuples.find(tree_name);
        if (it == m_ntuples.end() || !it->second.writer) return 0;
        return static_cast<long long>(it->second.writer->GetNEntries());
    }

    /**
     * @brief Print a summary of registered ntuples.
     */
    auto print_summary() const -> void {
        std::lock_guard lock(m_mutex);
        fmt::print("\nN-tuple Summary (RNTuple backend):\n");
        fmt::print("  Output file: {}\n", get_output_file());
        fmt::print("  Total ntuples: {}\n", m_ntuples.size());
        if (!m_ntuples.empty()) {
            fmt::print("\n  Ntuple Details:\n");
            for (const auto& [nm, pn] : m_ntuples) {
                const auto entries = pn.writer ? pn.writer->GetNEntries() : 0;
                const auto fields  = pn.copiers.size();
                fmt::print("    {:<20} : {} entries, {} fields\n", nm, entries, fields);
            }
        }
        fmt::print("\n");
    }

    /**
     * @brief Names of every registered ntuple.
     */
    auto get_tree_names() const -> std::vector<std::string> {
        std::lock_guard lock(m_mutex);
        std::vector<std::string> names;
        names.reserve(m_ntuples.size());
        for (const auto& [nm, pn] : m_ntuples) names.push_back(nm);
        return names;
    }

   private:
    /// Per-ntuple state. In the "pending" phase (before first fill)
    /// `model` is non-null and `copiers` accumulates per-field copy
    /// lambdas bound to user addresses. After the first fill the
    /// model is consumed by `RNTupleWriter::Append` and only
    /// `writer` + `copiers` remain live.
    struct PendingNtuple {
        std::unique_ptr<ROOT::RNTupleModel> model;
        std::vector<std::function<void()>> copiers;
        std::unique_ptr<ROOT::RNTupleWriter> writer;
        std::string title;
    };

    auto ensure_ntuple_locked(const std::string& name, const std::string& title) -> PendingNtuple& {
        auto [it, inserted] = m_ntuples.try_emplace(name);
        if (inserted) {
            it->second.model = ROOT::RNTupleModel::Create();
            it->second.title = title.empty() ? name : title;
        }
        return it->second;
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
            pn.writer = ROOT::RNTupleWriter::Append(std::move(pn.model),
                                                    std::string_view(tree_name),
                                                    *file);
        } catch (const std::exception& e) {
            Log::error(name(), "Failed to create RNTupleWriter for '{}': {}", tree_name, e.what());
            return false;
        }
        return true;
    }

    mutable std::mutex m_mutex;
    std::map<std::string, PendingNtuple> m_ntuples;
    OutputFileService* m_output_svc = nullptr;
};

/**
 * @brief RAII helper that calls `fill` on scope exit.
 *
 * Usage:
 * @code
 * {
 *     TreeFiller filler(ntuple_svc, "events");
 *     e_p = electron.p();
 *     e_theta = electron.theta();
 * }  // filler dtor flushes the entry
 * @endcode
 */
class TreeFiller {
   public:
    TreeFiller(NTupleService* service, std::string tree_name)
        : m_service(service), m_tree_name(std::move(tree_name)) {}

    ~TreeFiller() {
        if (m_service) m_service->fill(m_tree_name);
    }

    TreeFiller(const TreeFiller&) = delete;
    TreeFiller& operator=(const TreeFiller&) = delete;
    TreeFiller(TreeFiller&&) = delete;
    TreeFiller& operator=(TreeFiller&&) = delete;

   private:
    NTupleService* m_service;
    std::string m_tree_name;
};

}  // namespace clas12::framework
