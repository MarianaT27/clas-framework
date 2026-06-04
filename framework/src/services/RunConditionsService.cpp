/**
 * @file RunConditionsService.cpp
 * @brief SQLite-backed implementation of RunConditionsService.
 *
 * All use of the SQLite C API is confined to this translation unit; the public
 * header forward-declares `struct sqlite3` only. The schema queried is the
 * standard RCDB layout:
 *
 *   condition_types(id, name, value_type, ...)
 *   conditions(id, run_number, condition_type_id,
 *              float_value, int_value, text_value, bool_value, ...)
 *
 * A condition value lives in the column selected by `condition_types.value_type`
 * ("float" -> float_value, "int" -> int_value, "string"/"json" -> text_value,
 * "bool" -> bool_value).
 */

#include <clas12/framework/services/RunConditionsService.hpp>

#include <sqlite3.h>

#include <cstdlib>  // std::getenv
#include <optional>
#include <string>
#include <string_view>

#include <clas12/framework/services/LoggingService.hpp>  // framework::Log

namespace framework {

namespace {

/// Strip a leading RCDB `sqlite://` URL scheme, yielding a filesystem path.
///
/// Mirrors the SQLAlchemy convention used by RCDB:
///   sqlite:///example.db              -> example.db          (relative, 3 slashes)
///   sqlite:////group/clas12/x.db      -> /group/clas12/x.db  (absolute, 4 slashes)
/// A path with no scheme (e.g. "/group/clas12/x.db") is returned unchanged.
auto sqlite_path(std::string conn) -> std::string {
    constexpr std::string_view scheme = "sqlite://";
    if (std::string_view(conn).substr(0, scheme.size()) == scheme) {
        conn.erase(0, scheme.size());  // drop "sqlite://"
        // What remains is "/<path>"; the first '/' is the authority separator.
        // Dropping exactly one '/' turns "/example.db" -> "example.db" (relative)
        // and "//group/..." -> "/group/..." (absolute), matching SQLAlchemy.
        if (!conn.empty() && conn.front() == '/') conn.erase(0, 1);
    }
    return conn;
}

/// One row of the conditions/condition_types join for a (run, condition) pair.
struct CondRow {
    std::string value_type;
    bool has_float = false;
    double float_value = 0.0;
    bool has_int = false;
    long long int_value = 0;
    bool has_bool = false;
    long long bool_value = 0;
    bool has_text = false;
    std::string text_value;
};

/// Run the point query for one named condition of one run. Returns nullopt when
/// the run/condition pair is absent (or on any SQLite error — treated as "no
/// value", since RCDB is a soft fallback).
auto fetch(sqlite3* db, int run, const char* condition) -> std::optional<CondRow> {
    static constexpr const char* kSql =
        "SELECT ct.value_type, c.float_value, c.int_value, c.text_value, c.bool_value "
        "FROM conditions c "
        "JOIN condition_types ct ON ct.id = c.condition_type_id "
        "WHERE c.run_number = ?1 AND ct.name = ?2 "
        "LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, kSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }
    sqlite3_bind_int(stmt, 1, run);
    sqlite3_bind_text(stmt, 2, condition, -1, SQLITE_STATIC);

    std::optional<CondRow> out;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        CondRow row;
        if (const auto* vt = sqlite3_column_text(stmt, 0)) {
            row.value_type = reinterpret_cast<const char*>(vt);
        }
        if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
            row.has_float = true;
            row.float_value = sqlite3_column_double(stmt, 1);
        }
        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
            row.has_int = true;
            row.int_value = sqlite3_column_int64(stmt, 2);
        }
        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
            row.has_text = true;
            if (const auto* tv = sqlite3_column_text(stmt, 3)) {
                row.text_value = reinterpret_cast<const char*>(tv);
            }
        }
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
            row.has_bool = true;
            row.bool_value = sqlite3_column_int64(stmt, 4);
        }
        out = std::move(row);
    }
    sqlite3_finalize(stmt);
    return out;
}

}  // namespace

RunConditionsService::~RunConditionsService() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

auto RunConditionsService::on_initialize() -> StatusCode {
    // Resolve the connection: explicit configuration first, then environment.
    std::string conn = m_connection;
    if (conn.empty()) {
        if (const char* env = std::getenv("RCDB_CONNECTION")) conn = env;
    }

    if (conn.empty()) {
        Log::info(name(),
                  "No RCDB connection configured (set RCDB_CONNECTION or pass a "
                  "sqlite:///…db path); run conditions will come only from the "
                  "analysis config.");
        return StatusCode::SUCCESS();  // soft: service runs disabled
    }

    if (std::string_view(conn).substr(0, 8) == "mysql://") {
        Log::warning(name(),
                     "MySQL RCDB connection '{}' is not supported by the SQLite "
                     "snapshot reader; run conditions will come only from the "
                     "analysis config. Point RCDB_CONNECTION at a sqlite:///…db "
                     "snapshot to enable lookups.",
                     conn);
        return StatusCode::SUCCESS();
    }

    m_resolved = sqlite_path(conn);
    int rc = sqlite3_open_v2(m_resolved.c_str(), &m_db, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        Log::warning(name(),
                     "Could not open RCDB snapshot '{}': {}. Run conditions will "
                     "come only from the analysis config.",
                     m_resolved, m_db ? sqlite3_errmsg(m_db) : "out of memory");
        if (m_db) {
            sqlite3_close(m_db);
            m_db = nullptr;
        }
        return StatusCode::SUCCESS();  // soft failure — never abort a job over RCDB
    }

    Log::success(name(), "Opened RCDB snapshot '{}' (read-only).", m_resolved);
    return StatusCode::SUCCESS();
}

auto RunConditionsService::on_finalize() -> StatusCode {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
    return StatusCode::SUCCESS();
}

auto RunConditionsService::query_double(int run, const char* condition)
    -> std::optional<double> {
    auto row = fetch(m_db, run, condition);
    if (!row) return std::nullopt;
    // Prefer the column the schema says holds the value, but fall back across
    // numeric columns so a mis-typed condition still resolves.
    if (row->value_type == "float" && row->has_float) return row->float_value;
    if (row->value_type == "int" && row->has_int)
        return static_cast<double>(row->int_value);
    if (row->value_type == "bool" && row->has_bool)
        return static_cast<double>(row->bool_value);
    if (row->has_float) return row->float_value;
    if (row->has_int) return static_cast<double>(row->int_value);
    if (row->has_bool) return static_cast<double>(row->bool_value);
    return std::nullopt;
}

auto RunConditionsService::query_text(int run, const char* condition)
    -> std::optional<std::string> {
    auto row = fetch(m_db, run, condition);
    if (!row || !row->has_text || row->text_value.empty()) return std::nullopt;
    return row->text_value;
}

auto RunConditionsService::query(int run) -> RunConditions {
    RunConditions rc;
    rc.run = run;
    if (!m_db) return rc;  // disabled — every optional stays empty
    rc.beam_energy = query_double(run, "beam_energy");
    rc.torus = query_double(run, "torus_scale");
    rc.solenoid = query_double(run, "solenoid_scale");
    rc.target = query_text(run, "target");
    return rc;
}

auto RunConditionsService::conditions(int run) -> RunConditions {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_cache.find(run); it != m_cache.end()) {
        return it->second;
    }
    RunConditions rc = query(run);
    m_cache.emplace(run, rc);
    return rc;
}

}  // namespace framework
