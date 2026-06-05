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

#ifdef HAVE_MYSQL
#include <mysql.h>  // live RCDB MySQL/MariaDB client (optional build dependency)
#endif

#include <cstdio>   // std::snprintf
#include <cstdlib>  // std::getenv, std::strtod, std::strtoll
#include <cstring>  // std::strlen
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

#ifdef HAVE_MYSQL
/// Parsed `mysql://user[:password]@host[:port]/database` components.
struct MysqlConn {
    std::string  user, password, host, database;
    unsigned int port = 0;  ///< 0 => the client's default (3306)
};

/// Parse a mysql:// URL. Returns nullopt if it is not a mysql:// URL or is
/// missing the host or database.
auto parse_mysql_url(const std::string& conn) -> std::optional<MysqlConn> {
    constexpr std::string_view scheme = "mysql://";
    if (std::string_view(conn).substr(0, scheme.size()) != scheme) return std::nullopt;
    const std::string rest = conn.substr(scheme.size());
    MysqlConn out;
    std::string hostpart = rest;
    if (const auto at = rest.find('@'); at != std::string::npos) {
        const std::string userinfo = rest.substr(0, at);
        hostpart = rest.substr(at + 1);
        if (const auto colon = userinfo.find(':'); colon != std::string::npos) {
            out.user = userinfo.substr(0, colon);
            out.password = userinfo.substr(colon + 1);
        } else {
            out.user = userinfo;
        }
    }
    const auto slash = hostpart.find('/');
    if (slash == std::string::npos) return std::nullopt;  // database required
    out.database = hostpart.substr(slash + 1);
    const std::string hostport = hostpart.substr(0, slash);
    if (const auto colon = hostport.find(':'); colon != std::string::npos) {
        out.host = hostport.substr(0, colon);
        out.port = static_cast<unsigned int>(
            std::strtoul(hostport.c_str() + colon + 1, nullptr, 10));
    } else {
        out.host = hostport;
    }
    if (out.host.empty() || out.database.empty()) return std::nullopt;
    return out;
}

/// MySQL counterpart of fetch(): same point query, same soft-fail contract
/// (nullopt on absent pair or any error). The values arrive as text (MySQL
/// text protocol) and are parsed into the typed CondRow columns. run is an int
/// and condition is an internal constant, so the formatted query is injection-safe.
auto mysql_fetch(MYSQL* db, int run, const char* condition) -> std::optional<CondRow> {
    char sql[512];
    std::snprintf(sql, sizeof(sql),
        "SELECT ct.value_type, c.float_value, c.int_value, c.text_value, c.bool_value "
        "FROM conditions c JOIN condition_types ct ON ct.id = c.condition_type_id "
        "WHERE c.run_number = %d AND ct.name = '%s' LIMIT 1",
        run, condition);
    if (mysql_query(db, sql) != 0) return std::nullopt;
    MYSQL_RES* res = mysql_store_result(db);
    if (!res) return std::nullopt;
    std::optional<CondRow> out;
    if (MYSQL_ROW row = mysql_fetch_row(res)) {
        const unsigned long* len = mysql_fetch_lengths(res);
        CondRow cr;
        if (row[0]) cr.value_type = row[0];
        if (row[1]) { cr.has_float = true; cr.float_value = std::strtod(row[1], nullptr); }
        if (row[2]) { cr.has_int   = true; cr.int_value   = std::strtoll(row[2], nullptr, 10); }
        if (row[3]) { cr.has_text  = true;
            cr.text_value.assign(row[3], len ? len[3] : std::strlen(row[3])); }
        if (row[4]) { cr.has_bool  = true; cr.bool_value  = std::strtoll(row[4], nullptr, 10); }
        out = std::move(cr);
    }
    mysql_free_result(res);
    return out;
}
#endif  // HAVE_MYSQL

/// Backend-agnostic point query: dispatch to the live MySQL connection when
/// one is open, otherwise the SQLite snapshot.
auto fetch_any(sqlite3* sqlite_db, void* mysql_db, int run, const char* condition)
    -> std::optional<CondRow> {
#ifdef HAVE_MYSQL
    if (mysql_db) return mysql_fetch(static_cast<MYSQL*>(mysql_db), run, condition);
#else
    (void)mysql_db;
#endif
    return fetch(sqlite_db, run, condition);
}

}  // namespace

RunConditionsService::~RunConditionsService() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
#ifdef HAVE_MYSQL
    if (m_mysql) {
        mysql_close(static_cast<MYSQL*>(m_mysql));
        m_mysql = nullptr;
    }
#endif
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
#ifdef HAVE_MYSQL
        const auto parsed = parse_mysql_url(conn);
        if (!parsed) {
            Log::warning(name(),
                         "Malformed mysql:// connection '{}' (expected "
                         "mysql://user[:password]@host[:port]/database); run "
                         "conditions will come only from the analysis config.",
                         conn);
            return StatusCode::SUCCESS();
        }
        MYSQL* mysql = mysql_init(nullptr);
        if (!mysql) {
            Log::warning(name(), "mysql_init() failed; run conditions disabled.");
            return StatusCode::SUCCESS();
        }
        unsigned int timeout = 10;  // seconds — never hang a job on a slow/unreachable DB
        mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
        if (!mysql_real_connect(
                mysql, parsed->host.c_str(),
                parsed->user.empty() ? nullptr : parsed->user.c_str(),
                parsed->password.empty() ? nullptr : parsed->password.c_str(),
                parsed->database.c_str(), parsed->port, nullptr, 0)) {
            Log::warning(name(),
                         "Could not connect to RCDB MySQL {}@{}/{}: {}. Run "
                         "conditions will come only from the analysis config.",
                         parsed->user, parsed->host, parsed->database,
                         mysql_error(mysql));
            mysql_close(mysql);
            return StatusCode::SUCCESS();  // soft — never abort a job over RCDB
        }
        m_mysql = mysql;
        m_resolved = parsed->host + "/" + parsed->database;
        Log::success(name(), "Connected to live RCDB MySQL {}@{}:{}/{}.",
                     parsed->user, parsed->host,
                     parsed->port ? parsed->port : 3306u, parsed->database);
        return StatusCode::SUCCESS();
#else
        Log::warning(name(),
                     "MySQL RCDB connection '{}' needs a MySQL/MariaDB client at "
                     "build time (none was found); run conditions will come only "
                     "from the analysis config. Use a sqlite:///…db snapshot, or "
                     "rebuild with a MySQL client available.",
                     conn);
        return StatusCode::SUCCESS();
#endif
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
#ifdef HAVE_MYSQL
    if (m_mysql) {
        mysql_close(static_cast<MYSQL*>(m_mysql));
        m_mysql = nullptr;
    }
#endif
    return StatusCode::SUCCESS();
}

auto RunConditionsService::query_double(int run, const char* condition)
    -> std::optional<double> {
    auto row = fetch_any(m_db, m_mysql, run, condition);
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
    auto row = fetch_any(m_db, m_mysql, run, condition);
    if (!row || !row->has_text) return std::nullopt;
    // RCDB text conditions are operator-entered free-text and routinely carry
    // surrounding whitespace (e.g. target = "CxC " / "D2 "); trim it so
    // downstream string matching (string_to_target, ...) sees a clean value.
    // An all-whitespace / empty value is treated as "no value".
    const std::string& raw = row->text_value;
    const auto first = raw.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return std::nullopt;
    const auto last = raw.find_last_not_of(" \t\r\n");
    return raw.substr(first, last - first + 1);
}

auto RunConditionsService::query(int run) -> RunConditions {
    RunConditions rc;
    rc.run = run;
    if (!m_db && !m_mysql) return rc;  // no backend open — every optional stays empty
    // RCDB stores beam_energy in MeV (e.g. 10532.2); the framework works in
    // GeV, so convert. torus_scale / solenoid_scale are dimensionless signed
    // scales and are used as-is.
    if (auto be = query_double(run, "beam_energy")) rc.beam_energy = *be / 1000.0;
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
