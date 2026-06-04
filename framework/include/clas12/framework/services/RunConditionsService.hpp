#pragma once

/**
 * @file RunConditionsService.hpp
 * @brief Resolve per-run beam/target/field conditions from an RCDB snapshot.
 *
 * The CLAS12 Run Conditions Database (RCDB) records, per run number, the beam
 * energy, the target, and the torus/solenoid field scales used during data
 * taking. This service reads those values from a *SQLite snapshot* of RCDB
 * (e.g. `/group/clas12/rcdb/example.db`, or the RCDB URL form
 * `sqlite:////group/clas12/rcdb/example.db`). No MySQL client, no subprocess,
 * works offline.
 *
 * The values are used to fill conditions the analysis TOML left unspecified —
 * **the TOML config always wins; RCDB only fills the gaps.** Lookups are cached
 * per run number and mutex-guarded, so the service is safe to share across the
 * worker threads of a multi-threaded run and across a chain that spans several
 * runs (re-query on run change).
 *
 * Example:
 * @code
 * builder.add_service<framework::RunConditionsService>();   // RCDB_CONNECTION env
 * // ...
 * auto rc = svc->conditions(run);          // RunConditions for this run (cached)
 * if (!config.beam_energy_set && rc.beam_energy) use(*rc.beam_energy);
 * @endcode
 */

#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/StatusCode.hpp>

// Opaque forward declaration: keeps <sqlite3.h> confined to the .cpp so no
// translation unit that merely includes this header pays for (or links against
// the symbols of) the SQLite C API beyond the framework library itself.
struct sqlite3;

namespace framework {

/**
 * @brief Run-level conditions sourced from RCDB.
 *
 * Every field is `std::optional`: a condition may be absent for a given run, or
 * the snapshot may be unavailable entirely (offline / not configured). Callers
 * read a field only to fill a gap the analysis config left open.
 */
struct RunConditions {
    int run = -1;                       ///< run number these conditions describe
    std::optional<double> beam_energy;  ///< RCDB `beam_energy` [GeV]
    std::optional<double> torus;        ///< RCDB `torus_scale`    (signed, -1..1)
    std::optional<double> solenoid;     ///< RCDB `solenoid_scale` (signed, -1..1)
    std::optional<std::string> target;  ///< RCDB `target` (e.g. "LH2", "LD2")

    /// Field polarity = sign of the torus scale: -1, 0, or +1; nullopt if torus
    /// is unknown. (The CLAS12 convention encodes bending direction in the sign.)
    [[nodiscard]] auto polarity() const -> std::optional<int> {
        if (!torus) return std::nullopt;
        return (*torus < 0.0) ? -1 : (*torus > 0.0 ? 1 : 0);
    }

    /// True when at least one condition was resolved from the snapshot.
    [[nodiscard]] auto any() const -> bool {
        return beam_energy.has_value() || torus.has_value() ||
               solenoid.has_value() || target.has_value();
    }
};

/**
 * @brief Analysis-config overrides that take precedence over RCDB values.
 *
 * Each set optional is a value the analysis TOML specified explicitly; it wins
 * over whatever RCDB reports. An empty optional leaves the RCDB value in place.
 * This is the "TOML always wins; RCDB fills the gaps" merge, expressed as data.
 */
struct RunConditionsOverrides {
    std::optional<double> beam_energy;
    std::optional<std::string> target;
    std::optional<double> torus;     ///< signed torus scale; sign sets polarity
    std::optional<double> solenoid;  ///< signed solenoid scale
};

/// Merge config overrides onto RCDB-sourced conditions: a set override wins, an
/// unset one keeps the RCDB value. Pure function — the unit of the "TOML wins,
/// RCDB fills gaps" policy, independent of any database.
[[nodiscard]] inline auto apply_overrides(RunConditions base,
                                          const RunConditionsOverrides& ov)
    -> RunConditions {
    if (ov.beam_energy) base.beam_energy = ov.beam_energy;
    if (ov.target) base.target = ov.target;
    if (ov.torus) base.torus = ov.torus;
    if (ov.solenoid) base.solenoid = ov.solenoid;
    return base;
}

/**
 * @brief Service resolving per-run conditions from an RCDB SQLite snapshot.
 *
 * Connection resolution order (first non-empty wins):
 *   1. the string passed to the constructor / set_connection()
 *   2. the `RCDB_CONNECTION` environment variable
 *
 * Accepted forms: a plain filesystem path, or the RCDB URL
 * `sqlite:///relative.db` / `sqlite:////absolute/path.db`. A `mysql://…`
 * connection (the live JLab server) is **not** supported by this snapshot
 * reader: it is reported once and the service runs disabled — every lookup then
 * returns empty conditions, so TOML-specified values still flow unchanged.
 *
 * All failure modes are *soft*: a missing/unopenable snapshot logs a warning and
 * leaves the service disabled rather than aborting the job. RCDB is a
 * convenience fallback, never a hard dependency.
 */
class RunConditionsService : public Service {
   public:
    /// Stable name for service location (used by require_service<T>()).
    static constexpr auto service_name() -> std::string_view {
        return "RunConditionsService";
    }

    /// @param connection SQLite path or `sqlite:///…` URL; empty => RCDB_CONNECTION env.
    explicit RunConditionsService(std::string connection = {})
        : Service(std::string(service_name())),
          m_connection(std::move(connection)) {}

    ~RunConditionsService() override;  // closes the DB if on_finalize() did not run

    /// Override the connection string before initialize() (no effect afterwards).
    auto set_connection(std::string connection) -> void {
        m_connection = std::move(connection);
    }

    /**
     * @brief Conditions for @p run, cached and thread-safe.
     *
     * Returns by value (a copy of the cached entry) so the result stays valid if
     * another thread mutates the cache. A cache miss runs up to four point
     * queries against the snapshot. When the snapshot is unavailable, returns a
     * `RunConditions{run}` with every optional empty.
     */
    [[nodiscard]] auto conditions(int run) -> RunConditions;

    /// True when a SQLite snapshot was opened successfully.
    [[nodiscard]] auto is_open() const -> bool { return m_db != nullptr; }

    /// The filesystem path actually opened (empty when disabled).
    [[nodiscard]] auto connection() const -> const std::string& { return m_resolved; }

   private:
    auto on_initialize() -> StatusCode override;  // opens the SQLite snapshot
    auto on_finalize() -> StatusCode override;    // closes it

    // Implemented in RunConditionsService.cpp, where <sqlite3.h> is visible.
    auto query(int run) -> RunConditions;
    auto query_double(int run, const char* condition) -> std::optional<double>;
    auto query_text(int run, const char* condition) -> std::optional<std::string>;

    std::string m_connection;  ///< as configured (path or sqlite:// URL)
    std::string m_resolved;    ///< actual file path opened (post URL-strip)
    sqlite3* m_db = nullptr;   ///< opaque SQLite handle (opened READONLY)
    std::mutex m_mutex;        ///< guards m_db access + m_cache
    std::unordered_map<int, RunConditions> m_cache;
};

}  // namespace framework
