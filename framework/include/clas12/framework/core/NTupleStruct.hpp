#pragma once

/**
 * @file NTupleStruct.hpp
 * @brief Struct-based N-Tuple registration with automatic branch creation
 *
 * This file provides utilities for automatically registering N-Tuple branches
 * from struct fields, eliminating repetitive register_branch() calls.
 *
 * Example usage:
 * @code
 * // BEFORE: 5 lines of repetitive registration
 * svc->register_branch("tree", "e_p", &m_e_p);
 * svc->register_branch("tree", "e_theta", &m_e_theta);
 * svc->register_branch("tree", "e_phi", &m_e_phi);
 * svc->register_branch("tree", "e_vz", &m_e_vz);
 * svc->register_branch("tree", "e_chi2pid", &m_e_chi2pid);
 *
 * // AFTER: Struct-based registration
 * struct ElectronNTuple {
 *     double e_p = 0, e_theta = 0, e_phi = 0, e_vz = 0, e_chi2pid = 0;
 * };
 *
 * NTupleWriter<ElectronNTuple> m_ntuple;
 *
 * // In on_initialize():
 * m_ntuple.init(m_ntuple_svc->get(), "electron_kinematics");
 *
 * // In process():
 * m_ntuple.data().e_p = electron.p();
 * m_ntuple.fill();
 * @endcode
 */

#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <vector>

#include <clas12/framework/core/Field.hpp>
#include <clas12/framework/services/NTupleService.hpp>

namespace framework {

// =============================================================================
// Modern template-based field registration (preferred, no macros)
// =============================================================================
//
// A struct lists its N-tuple branches via a static constexpr member:
//
//     struct ElectronNTupleData {
//         double e_p = 0.0;
//         double e_theta = 0.0;
//         int    sector = 0;
//
//         static constexpr auto ntuple_fields() {
//             using T = ElectronNTupleData;
//             return std::make_tuple(
//                 framework::field("e_p",     &T::e_p),
//                 framework::field("e_theta", &T::e_theta),
//                 framework::field("sector",  &T::sector));
//         }
//     };
//
// NTupleWriter<T> picks up that static and wires each field into the
// service at `init()` time via a single fold-expression call.  No
// preprocessor, full type-checking, no 20-field limit.

/// Register every field listed in `fields` onto `svc` for `tree`.
///
/// Legacy helper, retained for source compatibility. The new NTupleWriter<T>
/// path drives schema registration through NTupleService::register_struct<T>
/// directly; this helper is unused but kept so any out-of-tree caller still
/// compiles.
template <typename T, typename Svc, typename... Fs>
auto register_fields(Svc* svc, const std::string& tree, T& /*data*/,
                     const std::tuple<Fs...>& /*fields*/) -> void {
    if (!svc) return;
    svc->template register_struct<T>(tree);
}


/**
 * @brief RAII wrapper for struct-based N-Tuple management
 *
 * NTupleWriter provides convenient management of N-Tuple data structs with:
 * - Automatic branch registration from struct fields
 * - Thread-safe filling
 * - Clean data() accessor for modification
 *
 * The template parameter T must expose a `static constexpr auto
 * ntuple_fields()` returning a `std::tuple` of `field("name", &T::member)`
 * entries.
 *
 * Example:
 * @code
 * struct ElectronData {
 *     double p = 0;
 *     double theta = 0;
 *     double phi = 0;
 *
 *     static constexpr auto ntuple_fields() {
 *         using T = ElectronData;
 *         return std::make_tuple(
 *             framework::field("p",     &T::p),
 *             framework::field("theta", &T::theta),
 *             framework::field("phi",   &T::phi));
 *     }
 * };
 *
 * class MyAlgorithm {
 *     NTupleWriter<ElectronData> m_electron_nt;
 *
 *     auto on_initialize() -> StatusCode {
 *         m_electron_nt.init(m_ntuple_svc->get(), "electrons");
 *         return StatusCode::SUCCESS();
 *     }
 *
 *     auto process(Event& event) -> StatusCode {
 *         m_electron_nt.data().p = electron.p();
 *         m_electron_nt.data().theta = electron.theta();
 *         m_electron_nt.data().phi = electron.phi();
 *         m_electron_nt.fill();
 *         return StatusCode::SUCCESS();
 *     }
 * };
 * @endcode
 *
 * @tparam T Struct exposing `static constexpr auto ntuple_fields()`.
 */
template <typename T>
class NTupleWriter {
    // NOTE: no class-level `static_assert(HasStaticNTupleFields<T>)` here.
    // When `NTupleWriter<T>` is declared as a member of T's enclosing
    // class, C++'s deferred-body-parsing rule means `T::ntuple_fields()`
    // hasn't been analysed yet and the concept check silently reports
    // `false`.  The static_assert inside `init()` runs after the enclosing
    // class is complete, so detection works reliably there.

public:
    NTupleWriter() = default;

    /**
     * @brief Construct and initialize with service and tree name
     *
     * @param svc N-Tuple service pointer
     * @param tree_name Name of the TTree
     */
    NTupleWriter(NTupleService* svc, const std::string& tree_name) {
        init(svc, tree_name);
    }

    /**
     * @brief Initialize the writer (register branches)
     *
     * Call this in on_initialize() after services are available.
     *
     * @param svc N-Tuple service pointer
     * @param tree_name Name of the TTree
     */
    auto init(NTupleService* svc, const std::string& tree_name) -> void {
        m_svc = svc;
        m_tree_name = tree_name;
        if (svc) {
            static_assert(HasStaticNTupleFields<T> || std::is_aggregate_v<T>,
                          "NTupleWriter<T>: T must either expose a static "
                          "constexpr auto ntuple_fields(), or be a plain "
                          "aggregate so its fields can be reflected via Boost.PFR");
            // Register the schema once; per-thread Slot<T> instances
            // (each carrying its own T storage + RNTupleFillContext +
            // pre-bound REntry) are lazily allocated on first access
            // from each worker thread via NTupleService::acquire_slot<T>.
            svc->template register_struct<T>(tree_name);
            m_initialized = true;
        }
    }

    /**
     * @brief Check if writer is initialized
     */
    [[nodiscard]] auto is_initialized() const -> bool { return m_initialized && m_svc != nullptr; }

    /**
     * @brief Access the per-thread data struct for modification.
     *
     * Each worker thread sees its own T instance, bound by raw pointer
     * to the thread's RNTupleFillContext entry. The first call from a
     * given thread takes a brief lock to allocate the slot; subsequent
     * calls hit a cached pointer.
     */
    auto data() -> T& { return slot().value; }

    /**
     * @brief Access the per-thread data struct (const).
     */
    [[nodiscard]] auto data() const -> const T& {
        return const_cast<NTupleWriter*>(this)->slot().value;
    }

    /**
     * @brief Fill the ntuple with the current per-thread data.
     *
     * Lock-free in steady state: each worker thread fills its own
     * RNTupleFillContext, which queues independent clusters into the
     * shared RNTupleParallelWriter. ROOT serializes only at cluster
     * commit time.
     */
    auto fill() -> void {
        if (m_svc && m_initialized) slot().fill();
    }

    /**
     * @brief Fill conditionally
     *
     * @param condition Only fill if condition is true
     */
    auto fill_if(bool condition) -> void {
        if (condition) {
            fill();
        }
    }

    /**
     * @brief Get the tree name
     */
    [[nodiscard]] auto tree_name() const -> const std::string& { return m_tree_name; }

    /**
     * @brief Get the underlying service
     */
    [[nodiscard]] auto service() const -> NTupleService* { return m_svc; }

    /**
     * @brief Implicit bool conversion for validity checking
     */
    explicit operator bool() const { return is_initialized(); }

private:
    auto slot() -> typename NTupleService::Slot<T>& {
        // Per-thread cache of NTupleWriter*-keyed slot pointers. Keyed
        // on `this` so two NTupleWriter<T> instances bound to different
        // tree names never alias. Each thread has its own map, so no
        // cross-thread synchronisation is needed; the only lock is taken
        // by NTupleService::acquire_slot on a miss (once per thread per
        // writer, lifetime of the run).
        thread_local std::unordered_map<const NTupleWriter*,
                                        typename NTupleService::Slot<T>*> cache;
        auto it = cache.find(this);
        if (it == cache.end()) {
            auto* s = m_svc->template acquire_slot<T>(m_tree_name);
            it = cache.emplace(this, s).first;
        }
        return *it->second;
    }

    NTupleService* m_svc = nullptr;
    std::string m_tree_name;
    bool m_initialized = false;
};


/**
 * @brief Helper function to create an NTupleWriter
 *
 * @tparam T Struct exposing `static constexpr auto ntuple_fields()`.
 * @param svc N-Tuple service pointer
 * @param tree_name Name of the TTree
 * @return NTupleWriter<T> Initialized writer
 */
template <typename T>
auto make_ntuple_writer(NTupleService* svc, const std::string& tree_name) -> NTupleWriter<T> {
    return NTupleWriter<T>(svc, tree_name);
}

}  // namespace framework
