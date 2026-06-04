/**
 * @file test_run_conditions.cpp
 * @brief Unit tests for RunConditionsService against a mock RCDB SQLite snapshot.
 *
 * Builds a tiny SQLite database mirroring the RCDB schema
 * (condition_types + conditions), then exercises the service: per-run lookup of
 * beam_energy / torus_scale / solenoid_scale / target, polarity derivation,
 * caching, the sqlite:// URL form, and the soft-failure paths (missing DB,
 * unsupported mysql:// connection).
 */

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <cstdio>
#include <string>

#include <clas12/framework/services/RunConditionsService.hpp>

namespace {

const std::string kMockDb = "/tmp/clas12_mock_rcdb_test.db";

// Write a minimal RCDB-schema SQLite snapshot with two physics runs:
//   run 810  : 6.423 GeV, torus -1, solenoid -1, target LH2  (inbending)
//   run 5038 : 10.6  GeV, torus +1, solenoid -1, target LD2  (outbending)
void make_mock_rcdb(const std::string& path) {
    std::remove(path.c_str());
    sqlite3* db = nullptr;
    ASSERT_EQ(sqlite3_open(path.c_str(), &db), SQLITE_OK);
    const char* sql =
        "CREATE TABLE condition_types ("
        "  id INTEGER PRIMARY KEY, name TEXT, value_type TEXT);"
        "CREATE TABLE conditions ("
        "  id INTEGER PRIMARY KEY, run_number INTEGER, condition_type_id INTEGER,"
        "  float_value REAL, int_value INTEGER, text_value TEXT, bool_value INTEGER);"
        "INSERT INTO condition_types (id,name,value_type) VALUES"
        "  (1,'beam_energy','float'),(2,'torus_scale','float'),"
        "  (3,'solenoid_scale','float'),(4,'target','string');"
        "INSERT INTO conditions (run_number,condition_type_id,float_value,text_value) VALUES"
        "  (810, 1, 6.423, NULL),(810, 2, -1.0, NULL),(810, 3, -1.0, NULL),(810, 4, NULL,'LH2'),"
        "  (5038,1, 10.6,  NULL),(5038,2,  1.0, NULL),(5038,3, -1.0, NULL),(5038,4, NULL,'LD2');";
    char* err = nullptr;
    const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    const std::string msg = err ? err : "";
    sqlite3_free(err);
    sqlite3_close(db);
    ASSERT_EQ(rc, SQLITE_OK) << msg;
}

}  // namespace

class RunConditionsTest : public ::testing::Test {
   protected:
    void SetUp() override { make_mock_rcdb(kMockDb); }
    void TearDown() override { std::remove(kMockDb.c_str()); }
};

TEST_F(RunConditionsTest, ResolvesRun810Inbending) {
    framework::RunConditionsService svc(kMockDb);
    ASSERT_TRUE(svc.initialize().is_success());
    EXPECT_TRUE(svc.is_open());

    const auto rc = svc.conditions(810);
    EXPECT_EQ(rc.run, 810);
    ASSERT_TRUE(rc.beam_energy.has_value());
    EXPECT_NEAR(*rc.beam_energy, 6.423, 1e-9);
    ASSERT_TRUE(rc.torus.has_value());
    EXPECT_NEAR(*rc.torus, -1.0, 1e-9);
    ASSERT_TRUE(rc.solenoid.has_value());
    EXPECT_NEAR(*rc.solenoid, -1.0, 1e-9);
    ASSERT_TRUE(rc.target.has_value());
    EXPECT_EQ(*rc.target, "LH2");
    ASSERT_TRUE(rc.polarity().has_value());
    EXPECT_EQ(*rc.polarity(), -1);  // torus < 0
    EXPECT_TRUE(rc.any());

    svc.finalize();
}

TEST_F(RunConditionsTest, ResolvesRun5038Outbending) {
    framework::RunConditionsService svc(kMockDb);
    ASSERT_TRUE(svc.initialize().is_success());

    const auto rc = svc.conditions(5038);
    EXPECT_NEAR(rc.beam_energy.value_or(0.0), 10.6, 1e-9);
    EXPECT_EQ(rc.target.value_or(""), "LD2");
    ASSERT_TRUE(rc.polarity().has_value());
    EXPECT_EQ(*rc.polarity(), 1);  // torus > 0

    svc.finalize();
}

TEST_F(RunConditionsTest, MissingRunReturnsEmpty) {
    framework::RunConditionsService svc(kMockDb);
    ASSERT_TRUE(svc.initialize().is_success());

    const auto rc = svc.conditions(999999);
    EXPECT_EQ(rc.run, 999999);
    EXPECT_FALSE(rc.beam_energy.has_value());
    EXPECT_FALSE(rc.target.has_value());
    EXPECT_FALSE(rc.polarity().has_value());
    EXPECT_FALSE(rc.any());

    svc.finalize();
}

TEST_F(RunConditionsTest, CachesRepeatedLookups) {
    framework::RunConditionsService svc(kMockDb);
    ASSERT_TRUE(svc.initialize().is_success());

    const auto a = svc.conditions(810);
    const auto b = svc.conditions(810);  // second hit served from cache
    EXPECT_EQ(a.beam_energy, b.beam_energy);
    EXPECT_EQ(a.target, b.target);

    svc.finalize();
}

TEST_F(RunConditionsTest, AcceptsSqliteUrlForm) {
    // RCDB absolute-path URL: "sqlite:///" + "/abs/path" -> "sqlite:////abs/path".
    framework::RunConditionsService svc("sqlite:///" + kMockDb);
    ASSERT_TRUE(svc.initialize().is_success());
    EXPECT_TRUE(svc.is_open());
    EXPECT_EQ(svc.connection(), kMockDb);  // URL scheme stripped back to the path
    EXPECT_NEAR(svc.conditions(810).beam_energy.value_or(0.0), 6.423, 1e-9);
    svc.finalize();
}

TEST(RunConditionsSoftFailure, MissingDatabaseIsSoft) {
    framework::RunConditionsService svc("/tmp/clas12_no_such_rcdb_snapshot.db");
    // A missing snapshot must never abort the job: initialize() still succeeds,
    // the service is simply disabled and every lookup comes back empty.
    EXPECT_TRUE(svc.initialize().is_success());
    EXPECT_FALSE(svc.is_open());
    EXPECT_FALSE(svc.conditions(810).any());
    svc.finalize();
}

TEST(RunConditionsSoftFailure, MysqlConnectionIsDisabled) {
    framework::RunConditionsService svc("mysql://rcdb@clasdb.jlab.org/rcdb");
    EXPECT_TRUE(svc.initialize().is_success());
    EXPECT_FALSE(svc.is_open());  // snapshot reader does not speak MySQL
    EXPECT_FALSE(svc.conditions(810).any());
    svc.finalize();
}

// ─── apply_overrides(): the "TOML always wins; RCDB fills gaps" merge ───────────

TEST(ApplyOverrides, TomlValuesWinOverRcdb) {
    framework::RunConditions rcdb;  // pretend these came from the snapshot
    rcdb.run = 810;
    rcdb.beam_energy = 6.423;
    rcdb.torus = -1.0;
    rcdb.solenoid = -1.0;
    rcdb.target = "LH2";

    framework::RunConditionsOverrides toml;  // analyst pinned beam energy + target
    toml.beam_energy = 10.6;
    toml.target = "LD2";

    const auto merged = framework::apply_overrides(rcdb, toml);
    EXPECT_NEAR(merged.beam_energy.value_or(0.0), 10.6, 1e-9);  // TOML wins
    EXPECT_EQ(merged.target.value_or(""), "LD2");               // TOML wins
    EXPECT_NEAR(merged.torus.value_or(0.0), -1.0, 1e-9);        // RCDB kept (no override)
    EXPECT_NEAR(merged.solenoid.value_or(0.0), -1.0, 1e-9);     // RCDB kept
}

TEST(ApplyOverrides, RcdbFillsGapsTomlLeftOpen) {
    framework::RunConditions rcdb;
    rcdb.beam_energy = 6.423;
    rcdb.target = "LH2";
    rcdb.torus = -1.0;

    const framework::RunConditionsOverrides empty;  // analyst specified nothing
    const auto merged = framework::apply_overrides(rcdb, empty);
    EXPECT_NEAR(merged.beam_energy.value_or(0.0), 6.423, 1e-9);  // all from RCDB
    EXPECT_EQ(merged.target.value_or(""), "LH2");
    ASSERT_TRUE(merged.polarity().has_value());
    EXPECT_EQ(*merged.polarity(), -1);  // derived from RCDB torus sign
}

TEST(ApplyOverrides, OverrideFillsWhenRcdbHasNothing) {
    const framework::RunConditions empty_rcdb;  // snapshot unavailable
    framework::RunConditionsOverrides toml;
    toml.beam_energy = 10.6;
    toml.torus = 1.0;

    const auto merged = framework::apply_overrides(empty_rcdb, toml);
    EXPECT_NEAR(merged.beam_energy.value_or(0.0), 10.6, 1e-9);
    EXPECT_FALSE(merged.target.has_value());  // neither side supplied a target
    ASSERT_TRUE(merged.polarity().has_value());
    EXPECT_EQ(*merged.polarity(), 1);  // from the torus override sign
}

// ─── Validation against a real RCDB snapshot (opt-in) ──────────────────────────
//
// Skipped unless RCDB_TEST_DB points at a real RCDB SQLite snapshot, so this is
// safe in CI. Run with:
//   RCDB_TEST_DB=/path/to/rcdb.db ./build/test/test_run_conditions
// Run 810 is a standard CLAS12 run: beam_energy 6.423 GeV, torus_scale -0.5.

TEST(RunConditionsRealDb, ReadsRun810FromSnapshot) {
    const char* db = std::getenv("RCDB_TEST_DB");
    if (!db || !*db)
        GTEST_SKIP() << "set RCDB_TEST_DB=/path/to/rcdb.db to validate against a "
                        "real RCDB snapshot";

    framework::RunConditionsService svc(db);
    ASSERT_TRUE(svc.initialize().is_success());
    if (!svc.is_open()) GTEST_SKIP() << "RCDB_TEST_DB could not be opened: " << db;

    const auto rc = svc.conditions(810);
    if (!rc.beam_energy)
        GTEST_SKIP() << "snapshot has no run 810; provide one that includes it";

    EXPECT_NEAR(*rc.beam_energy, 6.423, 1e-3);  // real CLAS12 value for run 810
    ASSERT_TRUE(rc.torus.has_value());
    EXPECT_NEAR(*rc.torus, -0.5, 1e-6);
    ASSERT_TRUE(rc.polarity().has_value());
    EXPECT_EQ(*rc.polarity(), -1);  // torus < 0 -> inbending

    // A run far outside the snapshot resolves to empty (soft, no error).
    EXPECT_FALSE(svc.conditions(2147483647).any());

    svc.finalize();
}
