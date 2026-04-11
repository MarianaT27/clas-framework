// Smoke test for the generated bank wrappers and PindexLinker.
//
// The goal is to prove that:
//   (a) every header in the umbrella compiles cleanly,
//   (b) the generated classes expose the API the rest of the framework is
//       going to call (bind, rows, row(i), per-column getters, link handles),
//   (c) PindexLinker accepts the handle exposed by a generated wrapper and
//       compiles through its template helper.
//
// We don't drive a real HIPO file here; that would require a fixture. All
// calls operate on default-constructed (invalid) bank_views, which is
// enough to exercise the API surface and catch compile-time regressions
// the moment someone breaks the generator or PindexLinker.

#include <gtest/gtest.h>

#include <clas12/bankdefs/analysis_banks.hpp>
// Pull in every generated wrapper so any regression in the generator
// (obscure column name, reserved keyword, duplicate type, etc.) fails the
// build instead of hiding behind "nobody includes this bank yet".
#include <clas12/bankdefs/all_banks.hpp>
#include <clas12/framework/core/PindexLinker.hpp>

#include <bank_view.h>

TEST(BankDefs, RECParticle_BindEmpty) {
    clas12::bankdefs::REC_Particle particle;
    hipo::bank_view empty{};
    EXPECT_FALSE(particle.bind(empty));
    EXPECT_FALSE(particle.valid());
    EXPECT_EQ(particle.rows(), 0);
}

TEST(BankDefs, RECCalorimeter_ExposesPindexHandle) {
    // The pindex column must be exposed with both a scalar getter and a
    // column_handle accessor so PindexLinker can bind to it.
    clas12::bankdefs::REC_Calorimeter calo;
    static_assert(std::is_same_v<decltype(calo.pindex(0)), int>,
                  "pindex() must return a widened int regardless of storage type");
    static_assert(
        std::is_same_v<decltype(calo.pindex_handle()), hipo::column_handle>,
        "pindex_handle() must return a hipo::column_handle");
}

TEST(BankDefs, RECParticle_RowViewHasExpectedGetters) {
    clas12::bankdefs::REC_Particle particle;
    // Row view is a lightweight pointer + index wrapper. Verify its type
    // surface without invoking methods on the invalid view.
    auto row = particle.row(0);
    (void)row;
    static_assert(sizeof(decltype(particle.row(0))) <=
                      sizeof(void*) + sizeof(int) * 2,
                  "Row view should be trivial");
}

TEST(BankDefs, PindexLinker_EmptyBankIsEmpty) {
    clas12::bankdefs::REC_Calorimeter calo;
    hipo::bank_view empty{};
    calo.bind(empty);

    auto linker = clas12::framework::link_by_pindex(calo);
    EXPECT_TRUE(linker.empty());
    EXPECT_EQ(linker.size(), 0u);
    EXPECT_TRUE(linker.rows_for(0).empty());
    EXPECT_TRUE(linker.rows_for(-1).empty());
}

TEST(BankDefs, PindexLinker_ManualBuildEmpty) {
    clas12::framework::PindexLinker linker;
    hipo::bank_view empty{};
    linker.build(empty, hipo::column_handle{});
    EXPECT_TRUE(linker.empty());
    EXPECT_TRUE(linker.rows_for(5).empty());
}

TEST(BankDefs, BankNameConstants) {
    EXPECT_EQ(clas12::bankdefs::REC_Particle::bank_name, "REC::Particle");
    EXPECT_EQ(clas12::bankdefs::REC_Calorimeter::bank_name, "REC::Calorimeter");
    EXPECT_EQ(clas12::bankdefs::MC_Particle::bank_name, "MC::Particle");
}
