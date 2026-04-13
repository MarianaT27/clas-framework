#pragma once

#include <bank_view.h>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/physics/ParticleUtils.hpp>
#include <clas12/framework/core/PindexLinker.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>

#include <any>
#include <array>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace clas12::framework {

/**
 * @brief Type alias for storing particles grouped by their PID type
 */
using ParticleMap = std::map<clas12::PIDs, std::vector<clas12::Particle>>;

/**
 * @brief Event Data Model for CLAS12 analysis
 *
 * This class provides a high-level interface to event data, abstracting
 * away the details of HIPO bank access. It follows the Event Data Model
 * pattern used in ATLAS (xAOD) and CMS (EDM).
 *
 * The Event class uses lazy evaluation - data is only loaded from HIPO
 * banks when explicitly requested.
 */
class Event {
   public:
    /**
     * @brief Construct an Event from a HIPO bank view set
     * @param banks Reference to the bank view set for this event
     */
    explicit Event(hipo::bank_view_set& banks)
        : m_banks(banks) {}

    /**
     * @brief Get all particles (lazy loading)
     * @return const ParticleMap& Map of PID to particle vectors
     */
    auto particles() -> const ParticleMap& {
        if (!m_particles_loaded) {
            load_particles();
        }
        return m_particles;
    }

    /**
     * @brief Get particles of a specific type
     * @param pid Particle ID
     * @return const std::vector<clas12::Particle>& Vector of particles
     */
    auto particles(clas12::PIDs pid) -> const std::vector<clas12::Particle>& {
        if (!m_particles_loaded) {
            load_particles();
        }
        auto it = m_particles.find(pid);
        if (it != m_particles.end()) {
            return it->second;
        }
        static const std::vector<clas12::Particle> empty;
        return empty;
    }

    /**
     * @brief Get all particles as a flat range-compatible view
     *
     * Returns a lazy-evaluated view of all particles across all PID types.
     * This view is compatible with C++20 range adaptors and can be composed
     * using the pipe operator with filters from clas12::framework::Ranges.
     *
     * The view uses lazy evaluation - particles are only accessed when
     * the view is iterated, and filtering operations are applied on-the-fly.
     *
     * @return A range view of all particles
     *
     * Example usage:
     * @code
     * // Get all electrons with momentum > 2 GeV in Forward Detector
     * auto electrons = event.all_particles()
     *     | Ranges::by_pid(clas12::PIDs::ELECTRON)
     *     | Ranges::by_min_momentum(2.0)
     *     | Ranges::in_detector(clas12::FD);
     *
     * for (const auto& e : electrons) {
     *     // Process electron
     * }
     * @endcode
     *
     * @see Requirements 3.1, 3.3
     */
    auto all_particles() {
        if (!m_particles_loaded) {
            load_particles();
        }
        // Flatten all particle vectors into a single view using std::views::join
        // This creates a lazy view that iterates over all particles across all PID types
        return m_particles
            | std::views::values  // Get just the vectors (not the PID keys)
            | std::views::join;   // Flatten into a single range
    }

    /**
     * @brief Get all particles as a flat vector
     *
     * Returns a vector containing all particles from all PID types.
     * Unlike all_particles(), this eagerly evaluates and copies all particles.
     *
     * Use this when you need to store the particles or iterate multiple times.
     * For single-pass filtering, prefer all_particles() for better performance.
     *
     * @return std::vector<clas12::Particle> Vector of all particles
     */
    auto all_particles_vector() -> std::vector<clas12::Particle> {
        if (!m_particles_loaded) {
            load_particles();
        }
        std::vector<clas12::Particle> result;
        for (const auto& [pid, particles] : m_particles) {
            result.insert(result.end(), particles.begin(), particles.end());
        }
        return result;
    }

    /**
     * @brief Get the underlying bank view set (e.g., for event serialization)
     * @return const hipo::bank_view_set& Reference to the bank view set
     */
    auto get_bank_view_set() const -> const hipo::bank_view_set& {
        return m_banks;
    }

    /**
     * @brief Get a HIPO bank view by name
     *
     * Returns an empty bank_view (0 rows) if the bank was not declared by any
     * algorithm — so optional banks remain safe to access unconditionally.
     *
     * @param bank_name Name of the bank (e.g., "REC::Particle")
     * @return const hipo::bank_view& Reference to the bank view
     */
    auto get_bank(const std::string& bank_name) -> const hipo::bank_view& {
        static const hipo::bank_view empty{};
        auto r = m_banks.get(bank_name);
        return r.has_value() ? r->get() : empty;
    }

    /**
     * @brief Get multiple banks at once using structured bindings
     *
     * Returns a tuple of const references to the requested bank views.
     *
     * @tparam Names Variadic string types for bank names
     * @param names Bank names (e.g., "REC::Particle", "REC::Calorimeter")
     * @return std::tuple of const bank_view references
     *
     * Example usage:
     * @code
     * auto [particles, calo, track] = event.banks(
     *     "REC::Particle", "REC::Calorimeter", "REC::Track"
     * );
     *
     * // Process particles
     * for (int i = 0; i < particles.get().rows(); ++i) {
     *     auto px = particles.get().get<float>("px", i);
     *     // ...
     * }
     * @endcode
     */
    template <typename... Names>
    auto banks(Names&&... names) {
        return std::make_tuple(std::cref(get_bank(std::forward<Names>(names)))...);
    }

    /**
     * @brief Check if a bank exists and has data
     * @param bank_name Name of the bank
     * @return true if bank exists and has rows
     */
    auto has_bank(const std::string& bank_name) -> bool {
        auto r = m_banks.get(bank_name);
        return r.has_value() && r->get().rows() > 0;
    }

    /**
     * @brief Get event start time
     * @return double Start time in ns
     */
    auto start_time() -> double {
        if (!m_start_time.has_value()) {
            load_event_info();
        }
        return m_start_time.value_or(0.0);
    }

    /**
     * @brief Get event number
     * @return long Event number
     */
    auto event_number() const -> long {
        return m_event_number;
    }

    /**
     * @brief Set event number
     * @param evt_num Event number
     */
    auto set_event_number(long evt_num) -> void {
        m_event_number = evt_num;
    }

    /**
     * @brief Get file index
     * @return int File index
     */
    auto file_index() const -> int {
        return m_file_index;
    }

    /**
     * @brief Set file index
     * @param file_idx File index
     */
    auto set_file_index(int file_idx) -> void {
        m_file_index = file_idx;
    }

    // ========================================================================
    // Type-safe event store API (compile-time type checking)
    // ========================================================================

    /**
     * @brief Store a value with a typed key (type-safe API)
     *
     * This method provides compile-time type safety by using TypedKey<T>.
     * Type mismatches are caught at compile time rather than runtime.
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @param value The value to store
     *
     * Example usage:
     * @code
     * event.store_typed(Keys::Q2, 4.5);
     * event.store_typed(Keys::Electrons, selected_electrons);
     * @endcode
     */
    template <typename T>
    auto store_typed(TypedKey<T> key, T value) -> void {
        m_typed_store.store(key, std::move(value));
    }

    /**
     * @brief Retrieve a value by typed key (type-safe API)
     *
     * Returns the stored value if it exists and the type matches.
     * Returns std::nullopt if the key doesn't exist or type mismatches.
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @return std::optional<T> The value, or nullopt if not found/type mismatch
     *
     * Example usage:
     * @code
     * auto q2 = event.retrieve_typed(Keys::Q2);
     * if (q2) {
     *     std::cout << "Q2 = " << *q2 << std::endl;
     * }
     * @endcode
     */
    template <typename T>
    auto retrieve_typed(TypedKey<T> key) const -> std::optional<T> {
        return m_typed_store.retrieve(key);
    }

    /**
     * @brief Check if a typed key exists in the store
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @return bool True if key exists with matching type
     */
    template <typename T>
    auto contains_typed(TypedKey<T> key) const -> bool {
        return m_typed_store.contains(key);
    }

    /**
     * @brief Check if an algorithm decision was passed (type-safe API)
     *
     * @param key The typed decision key (must be TypedKey<bool>)
     * @return bool True if the decision passed, false otherwise
     *
     * Example usage:
     * @code
     * if (event.passes_decision_typed(Keys::HasGoodElectron)) {
     *     // Process event with good electron
     * }
     * @endcode
     */
    auto passes_decision_typed(TypedKey<bool> key) -> bool {
        return m_typed_store.retrieve(key).value_or(false);
    }

    /**
     * @brief Set an algorithm decision flag (type-safe API)
     *
     * @param key The typed decision key (must be TypedKey<bool>)
     * @param passed Whether the decision passed
     */
    auto set_decision_typed(TypedKey<bool> key, bool passed) -> void {
        m_typed_store.store(key, passed);
    }

    /**
     * @brief Check a decision and retrieve data in one call
     *
     * Combines the common pattern of checking a boolean decision flag
     * and then retrieving associated data from the event store.
     * Returns nullopt if the decision fails or the data is not found.
     *
     * @tparam T The data type to retrieve
     * @param decision The decision key to check
     * @param data The data key to retrieve
     * @return std::optional<T> The data if decision passes and data exists
     *
     * @code
     * // BEFORE: 3-4 lines repeated everywhere
     * if (!event.passes_decision_typed(Decisions::HasGoodDISElectron)) return FILTERED();
     * auto electrons_opt = event.retrieve_typed(Keys::SelectedElectrons);
     * if (!electrons_opt || electrons_opt->empty()) return FILTERED();
     *
     * // AFTER: 1-2 lines
     * auto electrons = event.require_if(Decisions::HasGoodDISElectron, Keys::SelectedElectrons);
     * if (!electrons) return FILTERED();
     * @endcode
     */
    template <typename T>
    auto require_if(TypedKey<bool> decision, TypedKey<T> data) -> std::optional<T> {
        if (!passes_decision_typed(decision)) return std::nullopt;
        return retrieve_typed(data);
    }

    /**
     * @brief Get access to the underlying typed event store
     *
     * @return TypedEventStore& Reference to the typed store
     */
    auto typed_store() -> TypedEventStore& {
        return m_typed_store;
    }

    /**
     * @brief Get const access to the underlying typed event store
     *
     * @return const TypedEventStore& Const reference to the typed store
     */
    auto typed_store() const -> const TypedEventStore& {
        return m_typed_store;
    }

    // ========================================================================
    // Generated Bank Wrapper Cache
    // ========================================================================
    //
    // bank<T>() and rows_for_pindex<T>() provide a per-event cache of
    // bound generated wrappers (clas12::bankdefs::REC_Calorimeter, etc.)
    // and their associated PindexLinker instances. The first call for a
    // given bank type binds the wrapper from m_banks and builds the
    // linker; subsequent calls reuse both.
    //
    // Event is freshly stack-constructed per HIPO record in Application,
    // so the cache is implicitly per-event and never shared across
    // worker threads. There is no invalidation logic because there is
    // no state to carry over.

    /**
     * @brief Get a bound generated bank wrapper, caching on first use.
     *
     * The wrapper type T must be one of the classes generated under
     * clas12::bankdefs (e.g. REC_Calorimeter). bind() is called with the
     * hipo::bank_view looked up by T::bank_name on the first access, and
     * the bound wrapper is stashed for subsequent calls.
     *
     * @tparam T Generated bank wrapper type.
     * @return Reference to the bound wrapper.
     */
    template <typename T>
    auto bank() -> T& {
        auto& slot = m_bank_cache[std::type_index(typeid(T))];
        if (!slot.wrapper.has_value()) {
            T bound;
            // Generated readers (e.g. read_Calorimeter_bank) call bank<T>()
            // for sibling banks that the algorithm did not declare. Use the
            // checked accessor and fall back to an empty bank_view so the
            // wrapper's valid()/rows() return false instead of asserting.
            static const hipo::bank_view empty{};
            auto r = m_banks.get(std::string(T::bank_name));
            bound.bind(r.has_value() ? r->get() : empty);
            slot.wrapper = std::move(bound);
        }
        return std::any_cast<T&>(slot.wrapper);
    }

    /**
     * @brief Column-oriented view of a generated bank wrapper.
     *
     * Alias for `bank<T>()` documenting that the caller intends to read
     * whole columns via the generated `*_col()` span accessors rather
     * than row-by-row. The returned object is the same cached wrapper,
     * so repeated calls reuse the bind + linker state built by
     * `bank<T>()` / `rows_for_pindex<T>()`.
     *
     * @code
     * // Vectorised histogram fill over every REC::Calorimeter energy.
     * auto energies = event.column<REC_Calorimeter>().energy_col();
     * h_energy->fill_span(energies);
     * @endcode
     *
     * @tparam T Generated bank wrapper type.
     * @return Reference to the bound wrapper (same object `bank<T>()` returns).
     */
    template <typename T>
    auto column() -> T& {
        return bank<T>();
    }

    /**
     * @brief Return a lazy view of `T::Row` proxies whose pindex column
     *        matches the given particle index.
     *
     * On the first call for a given bank type, binds the wrapper via
     * bank<T>() and builds a PindexLinker from T::pindex_handle(). The
     * linker is stashed alongside the wrapper and reused for subsequent
     * calls - the whole pindex column is read via visit_column exactly
     * once per event per bank.
     *
     * The returned `PindexRowView<T>` yields `T::Row` proxies on
     * dereference; each Row is a lightweight (pointer + index) object
     * generated for every HIPO bank. Callers iterate with a range-for
     * loop and read column values directly off the row:
     *
     * @code
     * for (auto hit : event.rows_for_pindex<REC_Calorimeter>(pindex)) {
     *     if (hit.detector() != ECAL) continue;
     *     // hit.energy(), hit.layer(), hit.lu(), ...
     * }
     * @endcode
     *
     * @tparam T Generated bank wrapper type with `Row`, `row(int)` and
     *           `pindex_handle()` members (all emitted by gen_bankdefs.py).
     * @param pindex Particle index to look up.
     * @return Lazy row-proxy view (empty if no matching rows).
     */
    template <typename T>
    auto rows_for_pindex(int pindex) -> PindexRowView<T> {
        auto& bound = bank<T>();  // ensures wrapper is bound and cached
        auto& slot = m_bank_cache[std::type_index(typeid(T))];
        if (!slot.pindex_linker_built) {
            slot.pindex_linker.build(bound.view(), bound.pindex_handle());
            slot.pindex_linker_built = true;
        }
        return PindexRowView<T>{&bound, slot.pindex_linker.rows_for(pindex)};
    }

    /**
     * @brief Return a lazy view of `T::Row` proxies whose `index` column
     *        matches the given key.
     *
     * Symmetric to `rows_for_pindex<T>()` but keyed on `T::index_handle()`
     * instead of `T::pindex_handle()`. Useful for sibling-row links such
     * as FMT::Tracks -> REC::Track, where the `index` column holds a row
     * index into another bank. The cache holds an independent linker
     * slot per bank type so pindex and index lookups don't interfere.
     *
     * @tparam T Generated bank wrapper type exposing `Row`, `row(int)`
     *           and `index_handle()`.
     * @param key Row index of the target bank to look up.
     * @return Lazy row-proxy view (empty if no matching rows).
     */
    template <typename T>
    auto rows_for_index(int key) -> PindexRowView<T> {
        auto& bound = bank<T>();
        auto& slot = m_bank_cache[std::type_index(typeid(T))];
        if (!slot.index_linker_built) {
            slot.index_linker.build(bound.view(), bound.index_handle());
            slot.index_linker_built = true;
        }
        return PindexRowView<T>{&bound, slot.index_linker.rows_for(key)};
    }

   private:
    hipo::bank_view_set& m_banks;
    ParticleMap m_particles;
    bool m_particles_loaded = false;
    std::optional<double> m_start_time;
    long m_event_number = -1;
    int m_file_index = -1;
    TypedEventStore m_typed_store;

    /// Per-event cache of bound generated wrappers and their reverse indexes.
    /// The key is a std::type_index(typeid(T)) for the wrapper class. Two
    /// independent linker slots are held: one keyed by `pindex_handle()`
    /// (for the usual particle -> hit lookup) and one keyed by
    /// `index_handle()` (for sibling-row links like FMT::Tracks.index ->
    /// REC::Track row). Each is built lazily on first use.
    struct BankCacheEntry {
        std::any wrapper;                  ///< Holds a concrete T by value.
        PindexLinker pindex_linker;        ///< Built on first rows_for_pindex<T>.
        PindexLinker index_linker;         ///< Built on first rows_for_index<T>.
        bool pindex_linker_built = false;
        bool index_linker_built  = false;
    };
    std::unordered_map<std::type_index, BankCacheEntry> m_bank_cache;

    /**
     * @brief Load particles from REC::Particle bank
     *
     * Hot-path note: resolves every column handle exactly once before the
     * row loop so per-particle access is handle-based, never name-based.
     * For an event with N particles this replaces 11*N schema binary
     * searches with 11 + N typed reads through a pre-resolved handle.
     */
    auto load_particles() -> void {
        const auto& REC_Particle = m_banks["REC::Particle"];

        // Initialize particle vectors for common types
        static constexpr std::array<clas12::PIDs, 9> target_types = {
            clas12::PIDs::ELECTRON, clas12::PIDs::POSITRON,
            clas12::PIDs::PHOTON,
            clas12::PIDs::PROTON, clas12::PIDs::NEUTRON,
            clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS,
            clas12::PIDs::KPLUS, clas12::PIDs::KMINUS};

        for (const auto& type : target_types) {
            m_particles[type] = std::vector<clas12::Particle>();
        }

        const int n_rows = REC_Particle.rows();
        if (n_rows == 0) {
            m_particles_loaded = true;
            return;
        }

        // Resolve every column handle once up front. Anything missing
        // leaves the handle invalid; subsequent get<T>() calls on an
        // invalid handle return garbage, so we early-out if pid itself is
        // missing (which would mean we're not looking at a REC::Particle).
        const auto& schema = REC_Particle.get_schema();
        auto resolve = [&](std::string_view name) -> hipo::column_handle {
            auto h = schema.get_handle(name);
            return h ? *h : hipo::column_handle{};
        };

        const auto h_pid     = resolve("pid");
        if (!h_pid.valid()) {
            m_particles_loaded = true;
            return;
        }
        const auto h_status  = resolve("status");
        const auto h_charge  = resolve("charge");
        const auto h_px      = resolve("px");
        const auto h_py      = resolve("py");
        const auto h_pz      = resolve("pz");
        const auto h_vx      = resolve("vx");
        const auto h_vy      = resolve("vy");
        const auto h_vz      = resolve("vz");
        const auto h_vt      = resolve("vt");
        const auto h_beta    = resolve("beta");
        const auto h_chi2pid = resolve("chi2pid");

        for (int i = 0; i < n_rows; ++i) {
            const int pid = REC_Particle.get<int32_t>(h_pid, i);

            // Find matching particle type. target_types is small (9) and
            // sorted by selection frequency; linear scan is faster than a
            // hash or map lookup at this size.
            clas12::PIDs particle_type;
            bool found = false;
            for (const auto& type : target_types) {
                if (pid == static_cast<int>(type)) {
                    particle_type = type;
                    found = true;
                    break;
                }
            }
            if (!found) continue;

            const int status  = static_cast<int>(REC_Particle.get<int16_t>(h_status, i));
            const int charge  = static_cast<int>(REC_Particle.get<int8_t>(h_charge, i));
            const double px   = static_cast<double>(REC_Particle.get<float>(h_px, i));
            const double py   = static_cast<double>(REC_Particle.get<float>(h_py, i));
            const double pz   = static_cast<double>(REC_Particle.get<float>(h_pz, i));
            const double vx   = static_cast<double>(REC_Particle.get<float>(h_vx, i));
            const double vy   = static_cast<double>(REC_Particle.get<float>(h_vy, i));
            const double vz   = static_cast<double>(REC_Particle.get<float>(h_vz, i));
            const double vt   = static_cast<double>(REC_Particle.get<float>(h_vt, i));
            const double beta = static_cast<double>(REC_Particle.get<float>(h_beta, i));
            const double chi2pid = static_cast<double>(REC_Particle.get<float>(h_chi2pid, i));

            const double mass = clas12::physics::ParticleUtils::get_mass(pid);
            const double E = clas12::physics::ParticleUtils::compute_energy(px, py, pz, pid);

            m_particles[particle_type].emplace_back(
                pid, status, i, charge, mass,
                px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
        }

        m_particles_loaded = true;
    }

    auto load_event_info() -> void {
        const auto& REC_Event = m_banks["REC::Event"];
        if (REC_Event.rows() > 0) {
            m_start_time = static_cast<double>(REC_Event.get<float>("startTime", 0));
        }
    }
};

}  // namespace clas12::framework
