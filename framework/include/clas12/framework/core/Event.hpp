#pragma once

#include <bank.h>
#include <chain.h>
#include <clas12/Detectors.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/physics/ParticleUtils.hpp>
#include <clas12/framework/core/PindexLinker.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>

#include <any>
#include <array>
#include <atomic>
#include <cmath>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace framework {

/**
 * @brief Type alias for storing particles grouped by their PID type
 */
using ParticleMap = std::map<PIDs, std::vector<Particle>>;

namespace detail {

/// Compile-time-stable, unique index per generated bankdef wrapper type.
/// First access from any thread assigns the next id from a global counter,
/// so the value of `bank_type_id<T>()` is identical across all threads /
/// Event instances within a single process.
inline std::atomic<int>& bank_type_counter() {
    static std::atomic<int> counter{0};
    return counter;
}

template <typename T>
inline int bank_type_id() {
    static const int id = bank_type_counter().fetch_add(1, std::memory_order_relaxed);
    return id;
}

/// Cap on distinct bankdef wrapper types observed in one process. The
/// project ships 273 generated bankdefs but a typical analysis only
/// touches ~10. The fixed cap keeps Event::m_bank_cache stack-friendly
/// (one slot per index, ~80 bytes each).
inline constexpr int kMaxBankTypes = 64;

}  // namespace detail

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
     * @brief Construct an Event from the per-callback chain wrapper.
     *
     * The Event borrows the `hipo::chain_event` for the lifetime of the
     * lambda invocation. Banks read by `get_bank(name)` are pulled from
     * the wrapper lazily and cached for the rest of the event so repeated
     * lookups don't re-decode the record.
     */
    explicit Event(hipo::chain_event& evt)
        : m_chain_event(&evt) {}

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
     * @return const std::vector<Particle>& Vector of particles
     */
    auto particles(PIDs pid) -> const std::vector<Particle>& {
        if (!m_particles_loaded) {
            load_particles();
        }
        auto it = m_particles.find(pid);
        if (it != m_particles.end()) {
            return it->second;
        }
        return empty_particles();
    }

    // ========================================================================
    // Per-PID convenience accessors. Every one is a zero-cost forward to the
    // already-cached `m_particles` map built by `load_particles()`.
    // ========================================================================
    auto electrons()    -> const std::vector<Particle>& { return particles(PIDs::ELECTRON); }
    auto positrons()    -> const std::vector<Particle>& { return particles(PIDs::POSITRON); }
    auto photons()      -> const std::vector<Particle>& { return particles(PIDs::PHOTON); }
    auto protons()      -> const std::vector<Particle>& { return particles(PIDs::PROTON); }
    auto neutrons()     -> const std::vector<Particle>& { return particles(PIDs::NEUTRON); }
    auto pions_plus()   -> const std::vector<Particle>& { return particles(PIDs::PIPLUS); }
    auto pions_minus()  -> const std::vector<Particle>& { return particles(PIDs::PIMINUS); }
    auto kaons_plus()   -> const std::vector<Particle>& { return particles(PIDs::KPLUS); }
    auto kaons_minus()  -> const std::vector<Particle>& { return particles(PIDs::KMINUS); }

    /**
     * @brief Particles by integer PDG id (parallel to `particles(PIDs)`).
     */
    auto by_pid(int pid) -> const std::vector<Particle>& {
        return particles(static_cast<PIDs>(pid));
    }

    /**
     * @brief Particles with the given charge (-1, 0, or +1).
     *
     * Served from a flat 3-slot vector of vectors built in the same single
     * pass over REC::Particle as `m_particles`. No second scan, no map.
     * Anything outside [-1, +1] returns an empty vector.
     */
    auto by_charge(int charge) -> const std::vector<Particle>& {
        if (!m_particles_loaded) {
            load_particles();
        }
        if (charge < -1 || charge > 1) return empty_particles();
        return m_by_charge[charge + 1];
    }

    /**
     * @brief First electron whose |status| falls in @p status_range and
     *        whose status is negative with non-zero momentum.
     *
     * Default range covers the forward detector band. Single O(N) scan over
     * the cached `electrons()` vector — same cost as the old free function.
     */
    auto find_trigger_electron(
        std::pair<double, double> status_range = {FD, CD}
    ) -> std::optional<Particle> {
        for (const auto& e : electrons()) {
            const double abs_status = std::abs(e.status());
            if (e.status() < 0.0 && e.p() != 0.0 &&
                status_range.first <= abs_status && abs_status < status_range.second) {
                return e;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Get all particles as a flat range-compatible view
     *
     * Returns a lazy-evaluated view of all particles across all PID types.
     * This view is compatible with C++20 range adaptors and can be composed
     * using the pipe operator with filters from framework::Ranges.
     *
     * The view uses lazy evaluation - particles are only accessed when
     * the view is iterated, and filtering operations are applied on-the-fly.
     *
     * @return A range view of all particles
     *
     * Example usage:
     * @code
     * for (const auto& p : event.all_particles()) {
     *     if (p.pdg() != PIDs::ELECTRON) continue;
     *     if (p.p() < 2.0) continue;
     *     // ... process forward-detector electron
     * }
     * @endcode
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
     * @return std::vector<Particle> Vector of all particles
     */
    auto all_particles_vector() -> std::vector<Particle> {
        if (!m_particles_loaded) {
            load_particles();
        }
        std::vector<Particle> result;
        for (const auto& [pid, particles] : m_particles) {
            result.insert(result.end(), particles.begin(), particles.end());
        }
        return result;
    }

    /**
     * @brief Raw event buffer (for HIPO event-writer style passthrough).
     *
     * Used by the skim / filter analyses that replay passing events to a
     * downstream HIPO file via HipoFileService::write_event. Returns the
     * underlying hipo::event byte buffer; safe across the algorithm chain
     * because the chain_event the framework holds owns it for the
     * duration of the user lambda.
     */
    auto get_raw_event_buffer() const -> const std::vector<char>& {
        return m_chain_event->raw()->getEventBuffer();
    }

    /**
     * @brief Get a HIPO bank by name, cached for the rest of this event.
     *
     * Returns a `hipo::bank` with 0 rows when the bank is absent from
     * either the dictionary or this particular event — so optional banks
     * remain safe to access unconditionally.
     */
    auto get_bank(const std::string& bank_name) -> const hipo::bank& {
        auto it = m_raw_bank_cache.find(bank_name);
        if (it == m_raw_bank_cache.end()) {
            it = m_raw_bank_cache.emplace(bank_name, read_bank(bank_name)).first;
        }
        return it->second;
    }

    /**
     * @brief Get multiple banks at once using structured bindings
     *
     * Returns a tuple of const references to the requested bank views.
     *
     * @tparam Names Variadic string types for bank names
     * @param names Bank names (e.g., "REC::Particle", "REC::Calorimeter")
     * @return std::tuple of const hipo::bank references
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
        return get_bank(bank_name).getRows() > 0;
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
     * // BEFORE: 3 lines repeated everywhere
     * if (!event.passes_decision_typed(Decisions::HasGoodDISElectron)) return FILTERED();
     * auto electron_opt = event.retrieve_typed(Keys::TriggerElectron);
     * if (!electron_opt) return FILTERED();
     *
     * // AFTER: 1-2 lines
     * auto electron = event.require_if(Decisions::HasGoodDISElectron, Keys::TriggerElectron);
     * if (!electron) return FILTERED();
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
    // bound generated wrappers (bankdefs::REC_Calorimeter, etc.)
    // and their associated PindexLinker instances. The first call for a
    // given bank type binds the wrapper from the chain_event and builds the
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
     * bankdefs (e.g. REC_Calorimeter). bind() is called with the
     * hipo::bank looked up by T::bank_name on the first access, and
     * the bound wrapper is stashed for subsequent calls.
     *
     * @tparam T Generated bank wrapper type.
     * @return Reference to the bound wrapper.
     */
    template <typename T>
    auto bank() -> T& {
        auto& slot = m_bank_cache[detail::bank_type_id<T>()];
        if (!slot.has_value()) {
            slot.emplace();
            T bound;
            // Generated readers (e.g. read_Calorimeter_bank) call bank<T>()
            // for sibling banks that the algorithm did not declare. Reading
            // through get_bank() gives us a 0-row hipo::bank in that case,
            // so the wrapper's row count reports zero instead of asserting.
            bound.bind(get_bank(std::string(T::bank_name)));
            slot->wrapper = std::move(bound);
        }
        return std::any_cast<T&>(slot->wrapper);
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
        auto& slot = *m_bank_cache[detail::bank_type_id<T>()];
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
        auto& slot = *m_bank_cache[detail::bank_type_id<T>()];
        if (!slot.index_linker_built) {
            slot.index_linker.build(bound.view(), bound.index_handle());
            slot.index_linker_built = true;
        }
        return PindexRowView<T>{&bound, slot.index_linker.rows_for(key)};
    }

   private:
    static auto empty_particles() -> const std::vector<Particle>& {
        static const std::vector<Particle> s_empty;
        return s_empty;
    }

    hipo::chain_event* m_chain_event{nullptr};
    /// Lazy cache of raw HIPO banks (one per bank name). Populated on first
    /// `get_bank(name)` for the event; never invalidated because the Event
    /// is freshly stack-constructed per record in Application::run().
    std::unordered_map<std::string, hipo::bank> m_raw_bank_cache;
    ParticleMap m_particles;
    /// Charge-indexed view of REC::Particle, slot index = charge + 1 so
    /// charges -1/0/+1 map to slots 0/1/2. Built in the same single pass
    /// as `m_particles` to avoid a second scan or map allocation.
    std::array<std::vector<Particle>, 3> m_by_charge;
    bool m_particles_loaded = false;
    std::optional<double> m_start_time;
    long m_event_number = -1;
    int m_file_index = -1;
    TypedEventStore m_typed_store;

    /// Per-event cache of bound generated wrappers and their reverse indexes,
    /// addressed by `detail::bank_type_id<T>()`. Each slot is wrapped in
    /// `std::optional` so the array's destructor only does cheap bool
    /// checks for unpopulated slots.
    struct BankCacheEntry {
        std::any wrapper;                  ///< Holds a concrete T by value.
        PindexLinker pindex_linker;        ///< Built on first rows_for_pindex<T>.
        PindexLinker index_linker;         ///< Built on first rows_for_index<T>.
        bool pindex_linker_built = false;
        bool index_linker_built  = false;
    };
    std::array<std::optional<BankCacheEntry>, detail::kMaxBankTypes> m_bank_cache;

    /**
     * @brief Load particles from REC::Particle bank
     *
     * Hot-path note: resolves every column handle exactly once before the
     * row loop so per-particle access is handle-based, never name-based.
     * For an event with N particles this replaces 11*N schema binary
     * searches with 11 + N typed reads through a pre-resolved handle.
     */
    /// Fetch a HIPO bank from the underlying chain_event by name. Returns a
    /// 0-row bank if the dictionary doesn't contain the schema, so callers
    /// can probe optional banks without checking dictionary membership.
    auto read_bank(const std::string& bank_name) const -> hipo::bank {
        if (!m_chain_event) return hipo::bank{};
        // chain_event::getBank throws when the dictionary is missing the
        // schema; treating that as "no rows" matches the legacy semantics
        // of bank_view_set::get on an unknown name.
        try {
            return m_chain_event->getBank(bank_name);
        } catch (const std::exception&) {
            return hipo::bank{};
        }
    }

    auto load_particles() -> void {
        const auto& REC_Particle = get_bank("REC::Particle");

        // Initialize particle vectors for common types
        static constexpr std::array<PIDs, 9> target_types = {
            PIDs::ELECTRON, PIDs::POSITRON,
            PIDs::PHOTON,
            PIDs::PROTON, PIDs::NEUTRON,
            PIDs::PIPLUS, PIDs::PIMINUS,
            PIDs::KPLUS, PIDs::KMINUS};

        for (const auto& type : target_types) {
            m_particles[type] = std::vector<Particle>();
        }

        const int n_rows = REC_Particle.getRows();
        if (n_rows == 0) {
            m_particles_loaded = true;
            return;
        }

        // Resolve every column index once up front via getEntryOrder.
        // -1 means the column is missing from this schema (legacy code
        // tolerated that for everything but pid).
        auto& schema = const_cast<hipo::bank&>(REC_Particle).getSchema();
        auto resolve = [&](const char* name) -> int {
            return schema.exists(name) ? schema.getEntryOrder(name) : -1;
        };

        const int i_pid = resolve("pid");
        if (i_pid < 0) {
            m_particles_loaded = true;
            return;
        }
        const int i_status  = resolve("status");
        const int i_charge  = resolve("charge");
        const int i_px      = resolve("px");
        const int i_py      = resolve("py");
        const int i_pz      = resolve("pz");
        const int i_vx      = resolve("vx");
        const int i_vy      = resolve("vy");
        const int i_vz      = resolve("vz");
        const int i_vt      = resolve("vt");
        const int i_beta    = resolve("beta");
        const int i_chi2pid = resolve("chi2pid");

        for (int i = 0; i < n_rows; ++i) {
            const int pid = REC_Particle.getInt(i_pid, i);

            // Find matching particle type. target_types is small (9) and
            // sorted by selection frequency; linear scan is faster than a
            // hash or map lookup at this size.
            PIDs particle_type;
            bool found = false;
            for (const auto& type : target_types) {
                if (pid == static_cast<int>(type)) {
                    particle_type = type;
                    found = true;
                    break;
                }
            }
            if (!found) continue;

            const int status  = i_status  >= 0 ? REC_Particle.getShort(i_status, i) : 0;
            const int charge  = i_charge  >= 0 ? REC_Particle.getByte(i_charge, i)  : 0;
            const double px   = i_px      >= 0 ? static_cast<double>(REC_Particle.getFloat(i_px, i))   : 0.0;
            const double py   = i_py      >= 0 ? static_cast<double>(REC_Particle.getFloat(i_py, i))   : 0.0;
            const double pz   = i_pz      >= 0 ? static_cast<double>(REC_Particle.getFloat(i_pz, i))   : 0.0;
            const double vx   = i_vx      >= 0 ? static_cast<double>(REC_Particle.getFloat(i_vx, i))   : 0.0;
            const double vy   = i_vy      >= 0 ? static_cast<double>(REC_Particle.getFloat(i_vy, i))   : 0.0;
            const double vz   = i_vz      >= 0 ? static_cast<double>(REC_Particle.getFloat(i_vz, i))   : 0.0;
            const double vt   = i_vt      >= 0 ? static_cast<double>(REC_Particle.getFloat(i_vt, i))   : 0.0;
            const double beta = i_beta    >= 0 ? static_cast<double>(REC_Particle.getFloat(i_beta, i)) : 0.0;
            const double chi2pid = i_chi2pid >= 0 ? static_cast<double>(REC_Particle.getFloat(i_chi2pid, i)) : 0.0;

            const double mass = physics::ParticleUtils::get_mass(pid);
            const double E = physics::ParticleUtils::compute_energy(px, py, pz, pid);

            auto& p = m_particles[particle_type].emplace_back(
                pid, status, i, charge, mass,
                px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
            p.set_event(this);

            // Charge index, populated in the same pass. Slot 0 = -1, 1 = 0,
            // 2 = +1. Anything outside that range is dropped (matches the
            // sign-of-charge model REC::Particle uses). The pushed copy
            // inherits `m_event` from `p`, no separate set_event needed.
            if (charge >= -1 && charge <= 1) {
                m_by_charge[charge + 1].push_back(p);
            }
        }

        m_particles_loaded = true;
    }

    auto load_event_info() -> void {
        const auto& REC_Event = get_bank("REC::Event");
        if (REC_Event.getRows() > 0) {
            m_start_time = static_cast<double>(REC_Event.getFloat("startTime", 0));
        }
    }
};

}  // namespace framework
