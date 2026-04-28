// Boundary tests for ElectronCutsService cut predicates.
//
// Covers every pure-function cut on the service: chi2pid, momentum,
// PCAL fiducial, DC edge, sampling-fraction-with-explicit-mean-sigma,
// and per-target Vz windows. Intentionally skips the variant of
// pass_sampling_fraction_cut that resolves through SamplingFractionService
// — that path is exercised by the integration tests once a service
// instance is wired up.

#include <gtest/gtest.h>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>

using framework::ElectronCutsService;

namespace {

auto make_default_service() -> ElectronCutsService {
    ElectronCutsService svc;
    // Defaults: chi2pid in (-5, 5), p > 0.8, pcal Loose (9 cm),
    // n_sigma = 3.5, target = LD2.
    return svc;
}

}  // namespace

// ---- chi2pid ---------------------------------------------------------------

TEST(ElectronCutsServiceTest, Chi2PidPassesInsideRange) {
    auto svc = make_default_service();
    EXPECT_TRUE(svc.pass_chi2pid_cut(0.0));
    EXPECT_TRUE(svc.pass_chi2pid_cut(-4.99));
    EXPECT_TRUE(svc.pass_chi2pid_cut(4.99));
}

TEST(ElectronCutsServiceTest, Chi2PidRejectsAtAndOutsideBoundary) {
    auto svc = make_default_service();
    // Strict inequality on both ends (min < x && x < max).
    EXPECT_FALSE(svc.pass_chi2pid_cut(-5.0));
    EXPECT_FALSE(svc.pass_chi2pid_cut(5.0));
    EXPECT_FALSE(svc.pass_chi2pid_cut(-10.0));
    EXPECT_FALSE(svc.pass_chi2pid_cut(10.0));
}

TEST(ElectronCutsServiceTest, Chi2PidHonoursCustomRange) {
    auto svc = make_default_service();
    svc.set_chi2pid_range(-2.0, 2.0);
    EXPECT_TRUE(svc.pass_chi2pid_cut(0.0));
    EXPECT_FALSE(svc.pass_chi2pid_cut(2.5));
    EXPECT_FALSE(svc.pass_chi2pid_cut(-2.5));
}

// ---- momentum --------------------------------------------------------------

TEST(ElectronCutsServiceTest, MomentumStrictGreaterThan) {
    auto svc = make_default_service();
    EXPECT_TRUE(svc.pass_momentum_cut(0.81));
    EXPECT_TRUE(svc.pass_momentum_cut(5.0));
    // p == min should fail (strict >).
    EXPECT_FALSE(svc.pass_momentum_cut(0.8));
    EXPECT_FALSE(svc.pass_momentum_cut(0.0));
    EXPECT_FALSE(svc.pass_momentum_cut(-1.0));
}

TEST(ElectronCutsServiceTest, MomentumHonoursCustomMin) {
    auto svc = make_default_service();
    svc.set_min_electron_momentum(2.0);
    EXPECT_TRUE(svc.pass_momentum_cut(2.01));
    EXPECT_FALSE(svc.pass_momentum_cut(1.99));
}

// ---- PCAL fiducial ---------------------------------------------------------

TEST(ElectronCutsServiceTest, PcalFiducialDefaultsToLoose9cm) {
    auto svc = make_default_service();
    // Loose level => 9 cm cut, strict greater-than.
    EXPECT_TRUE(svc.pass_pcal_v_cut(9.01));
    EXPECT_TRUE(svc.pass_pcal_w_cut(9.01));
    EXPECT_TRUE(svc.pass_pcal_fiducial_cut(9.01, 9.01));
    EXPECT_FALSE(svc.pass_pcal_v_cut(9.0));
    EXPECT_FALSE(svc.pass_pcal_fiducial_cut(9.01, 8.99));
    EXPECT_FALSE(svc.pass_pcal_fiducial_cut(8.99, 9.01));
}

TEST(ElectronCutsServiceTest, PcalFiducialMediumAndTightHardenCut) {
    auto svc = make_default_service();
    svc.set_pcal_fiducial_level(ElectronCutsService::PCALFiducialLevel::Medium);
    EXPECT_TRUE(svc.pass_pcal_fiducial_cut(14.01, 14.01));
    EXPECT_FALSE(svc.pass_pcal_fiducial_cut(13.99, 14.01));

    svc.set_pcal_fiducial_level(ElectronCutsService::PCALFiducialLevel::Tight);
    EXPECT_TRUE(svc.pass_pcal_fiducial_cut(19.01, 19.01));
    EXPECT_FALSE(svc.pass_pcal_fiducial_cut(18.99, 19.01));
}

// ---- DC edge cuts ----------------------------------------------------------

TEST(ElectronCutsServiceTest, DcEdgeCutEnforcesAllThreeRegions) {
    // Static method, no instance needed. Limits: R1 > 1.680, R2 > 2.0, R3 > 8.750.
    EXPECT_TRUE(ElectronCutsService::pass_dc_edge_cut(1.7, 2.1, 8.8));
    EXPECT_TRUE(ElectronCutsService::pass_dc_edge_cut(10.0, 10.0, 10.0));

    // Boundary exactly => fail.
    EXPECT_FALSE(ElectronCutsService::pass_dc_edge_cut(1.680, 2.1, 8.8));
    EXPECT_FALSE(ElectronCutsService::pass_dc_edge_cut(1.7, 2.0, 8.8));
    EXPECT_FALSE(ElectronCutsService::pass_dc_edge_cut(1.7, 2.1, 8.750));
}

TEST(ElectronCutsServiceTest, DcEdgeCutStructDelegatesToPasses) {
    ElectronCutsService::EdgeCuts good{2.0, 3.0, 9.0};
    EXPECT_TRUE(ElectronCutsService::pass_dc_edge_cut(good));

    ElectronCutsService::EdgeCuts bad{1.0, 3.0, 9.0};
    EXPECT_FALSE(ElectronCutsService::pass_dc_edge_cut(bad));
}

// ---- sampling fraction (explicit mean/sigma path, no service) -------------

TEST(ElectronCutsServiceTest, SamplingFractionExplicitNSigmaWindow) {
    auto svc = make_default_service();
    // Default n_sigma is 3.5. Mean=0.25, sigma=0.02 -> window (0.18, 0.32).
    EXPECT_TRUE(svc.pass_sampling_fraction_cut(0.25, 0.25, 0.02));
    EXPECT_TRUE(svc.pass_sampling_fraction_cut(0.31, 0.25, 0.02));
    EXPECT_FALSE(svc.pass_sampling_fraction_cut(0.32, 0.25, 0.02));  // boundary
    EXPECT_FALSE(svc.pass_sampling_fraction_cut(0.10, 0.25, 0.02));
    EXPECT_FALSE(svc.pass_sampling_fraction_cut(0.50, 0.25, 0.02));
}

TEST(ElectronCutsServiceTest, SamplingFractionTighterNSigmaShrinksWindow) {
    auto svc = make_default_service();
    svc.set_n_sigma(2.0);
    // window (0.21, 0.29)
    EXPECT_TRUE(svc.pass_sampling_fraction_cut(0.25, 0.25, 0.02));
    EXPECT_FALSE(svc.pass_sampling_fraction_cut(0.30, 0.25, 0.02));
}

// ---- Vz ranges (per-target) -----------------------------------------------

TEST(ElectronCutsServiceTest, VzRangePerTargetMatchesDefinedTable) {
    // Static: get_vz_range(target) returns RG-D ranges.
    auto ld2 = ElectronCutsService::get_vz_range(Target::LD2);
    EXPECT_DOUBLE_EQ(ld2.min, -15.0);
    EXPECT_DOUBLE_EQ(ld2.max, 5.0);

    auto cu = ElectronCutsService::get_vz_range(Target::Cu);
    EXPECT_DOUBLE_EQ(cu.min, -10.6);
    EXPECT_DOUBLE_EQ(cu.max, -6.5);
}

TEST(ElectronCutsServiceTest, VzCutUsesConfiguredTarget) {
    auto svc = make_default_service();
    svc.set_target(Target::Cu);
    // Cu range: (-10.6, -6.5), strict on both ends.
    EXPECT_TRUE(svc.pass_vz_cut(-8.0));
    EXPECT_FALSE(svc.pass_vz_cut(-10.6));  // boundary
    EXPECT_FALSE(svc.pass_vz_cut(-6.5));   // boundary
    EXPECT_FALSE(svc.pass_vz_cut(0.0));
}

TEST(ElectronCutsServiceTest, VzCutTargetOverloadIgnoresConfig) {
    auto svc = make_default_service();
    svc.set_target(Target::LD2);
    // LD2 range (-15, 5) contains 0; Cu range (-10.6, -6.5) does not.
    // Overload should pick the explicitly passed target, not config.
    EXPECT_TRUE(svc.pass_vz_cut(0.0));
    EXPECT_FALSE(svc.pass_vz_cut(0.0, Target::Cu));
}

// ---- valid sector helper ---------------------------------------------------

TEST(ElectronCutsServiceTest, IsValidSectorAccepts1To6Only) {
    EXPECT_TRUE(ElectronCutsService::is_valid_sector(1));
    EXPECT_TRUE(ElectronCutsService::is_valid_sector(6));
    EXPECT_FALSE(ElectronCutsService::is_valid_sector(0));
    EXPECT_FALSE(ElectronCutsService::is_valid_sector(7));
    EXPECT_FALSE(ElectronCutsService::is_valid_sector(-1));
}
