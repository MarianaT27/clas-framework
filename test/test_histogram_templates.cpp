#include <gtest/gtest.h>
#include <cstdio>
#include <filesystem>
#include <clas12/framework/core/HistogramTemplates.hpp>
#include <clas12/framework/core/SectorHistograms.hpp>
#include <clas12/framework/core/CutflowHistograms.hpp>
#include <clas12/framework/core/NTupleWriter.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>
#include "TFile.h"

using namespace clas12::framework;

// Test fixture for histogram template tests
class HistogramTemplatesTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Create a unique test output file
        m_output_file = "test_histogram_templates_output.root";

        // Set up OutputFileService (required by HistogramService)
        m_output_svc.set_output_file(m_output_file);
        m_output_svc.initialize();

        // Configure HistogramService with OutputFileService
        m_hist_svc.set_output_file_service(&m_output_svc);
        m_hist_svc.set_auto_write(true);
        m_hist_svc.set_verification(true);
        m_hist_svc.initialize();
    }

    void TearDown() override {
        // Clean up services
        m_hist_svc.finalize();
        m_output_svc.finalize();

        // Clean up test output file
        if (std::filesystem::exists(m_output_file)) {
            std::filesystem::remove(m_output_file);
        }
    }

    OutputFileService m_output_svc;
    HistogramService m_hist_svc;
    std::string m_output_file;
};

// ============================================================================
// HistSpec Tests
// ============================================================================

TEST_F(HistogramTemplatesTest, HistSpec1DCreation) {
    auto spec = HistSpec::hist1D("/Test/momentum", "Momentum;p [GeV/c];Events", 100, 0.0, 10.0);

    EXPECT_EQ(spec.path, "/Test/momentum");
    EXPECT_EQ(spec.title, "Momentum;p [GeV/c];Events");
    EXPECT_EQ(spec.nbins, 100);
    EXPECT_DOUBLE_EQ(spec.xmin, 0.0);
    EXPECT_DOUBLE_EQ(spec.xmax, 10.0);
    EXPECT_FALSE(spec.is_2d());
}

TEST_F(HistogramTemplatesTest, HistSpec2DCreation) {
    auto spec = HistSpec::hist2D("/Test/theta_phi", "Theta vs Phi", 
                                  360, -180.0, 180.0, 180, 0.0, 180.0);

    EXPECT_EQ(spec.path, "/Test/theta_phi");
    EXPECT_TRUE(spec.is_2d());
    EXPECT_EQ(spec.nbins, 360);
    EXPECT_EQ(*spec.nbinsy, 180);
    EXPECT_DOUBLE_EQ(*spec.ymin, 0.0);
    EXPECT_DOUBLE_EQ(*spec.ymax, 180.0);
}

// ============================================================================
// SectorHistograms Tests
// ============================================================================

TEST_F(HistogramTemplatesTest, SectorHistogramsCreation) {
    SectorHistograms<6> h_momentum(m_hist_svc, "/Electron/momentum",
                                    "Electron Momentum", 100, 0.0, 10.0);

    EXPECT_EQ(h_momentum.num_sectors(), 6);
    EXPECT_EQ(h_momentum.base_path(), "/Electron/momentum");

    // Verify all sector histograms were created
    for (int s = 1; s <= 6; ++s) {
        EXPECT_NE(h_momentum[s], nullptr) << "Sector " << s << " histogram is null";
    }
}

TEST_F(HistogramTemplatesTest, SectorHistogramsFill) {
    SectorHistograms<6> h_momentum(m_hist_svc, "/Electron/momentum",
                                    "Electron Momentum", 100, 0.0, 10.0);

    // Fill each sector with different values
    for (int s = 1; s <= 6; ++s) {
        h_momentum.fill(s, 5.0 + s * 0.1);
    }

    // Verify fills
    for (int s = 1; s <= 6; ++s) {
        EXPECT_EQ(h_momentum[s]->entries(), 1) << "Sector " << s << " should have 1 entry";
    }
}

TEST_F(HistogramTemplatesTest, SectorHistogramsFromSpec) {
    auto spec = HistSpec::hist1D("/Electron/theta", "Electron Theta", 180, 0.0, 180.0);
    SectorHistograms<6> h_theta(m_hist_svc, spec);

    EXPECT_EQ(h_theta.num_sectors(), 6);
    for (int s = 1; s <= 6; ++s) {
        EXPECT_NE(h_theta[s], nullptr);
    }
}

TEST_F(HistogramTemplatesTest, SectorHistogramsInvalidSector) {
    SectorHistograms<6> h_momentum(m_hist_svc, "/Electron/momentum",
                                    "Electron Momentum", 100, 0.0, 10.0);

    // Fill invalid sector should not crash
    h_momentum.fill(0, 5.0);   // Invalid
    h_momentum.fill(7, 5.0);   // Invalid
    h_momentum.fill(-1, 5.0);  // Invalid

    // Valid sectors should still work
    h_momentum.fill(1, 5.0);
    EXPECT_EQ(h_momentum[1]->entries(), 1);
}


// ============================================================================
// CutStageHistograms Tests
// ============================================================================

TEST_F(HistogramTemplatesTest, CutStageHistogramsCreation) {
    std::vector<std::string> cuts = {"All", "Fiducial", "PID", "Momentum"};
    CutStageHistograms h_momentum(m_hist_svc, "/Electron/cutflow/momentum",
                                   "Electron Momentum", 100, 0.0, 10.0, cuts);

    EXPECT_EQ(h_momentum.cut_names().size(), 4);
    EXPECT_EQ(h_momentum.base_path(), "/Electron/cutflow/momentum");

    // Verify all cut stage histograms were created
    for (const auto& cut : cuts) {
        EXPECT_NE(h_momentum[cut], nullptr) << "Cut stage '" << cut << "' histogram is null";
    }
}

TEST_F(HistogramTemplatesTest, CutStageHistogramsFill) {
    std::vector<std::string> cuts = {"All", "Fiducial", "PID"};
    CutStageHistograms h_momentum(m_hist_svc, "/Electron/cutflow/momentum",
                                   "Electron Momentum", 100, 0.0, 10.0, cuts);

    // Simulate cutflow
    h_momentum.fill_after_cut("All", 5.0);
    h_momentum.fill_after_cut("Fiducial", 5.0);
    // PID cut fails, so we don't fill

    EXPECT_EQ(h_momentum["All"]->entries(), 1);
    EXPECT_EQ(h_momentum["Fiducial"]->entries(), 1);
    EXPECT_EQ(h_momentum["PID"]->entries(), 0);
}

TEST_F(HistogramTemplatesTest, CutStageHistogramsFromSpec) {
    auto spec = HistSpec::hist1D("/Electron/cutflow/theta", "Electron Theta", 180, 0.0, 180.0);
    std::vector<std::string> cuts = {"All", "Selected"};
    CutStageHistograms h_theta(m_hist_svc, spec, cuts);

    EXPECT_EQ(h_theta.cut_names().size(), 2);
    for (const auto& cut : cuts) {
        EXPECT_NE(h_theta[cut], nullptr);
    }
}

TEST_F(HistogramTemplatesTest, CutStageHistogramsInvalidCut) {
    std::vector<std::string> cuts = {"All", "Selected"};
    CutStageHistograms h_momentum(m_hist_svc, "/Electron/cutflow/momentum",
                                   "Electron Momentum", 100, 0.0, 10.0, cuts);

    // Fill invalid cut should not crash
    h_momentum.fill_after_cut("NonExistent", 5.0);

    // Valid cuts should still work
    h_momentum.fill_after_cut("All", 5.0);
    EXPECT_EQ(h_momentum["All"]->entries(), 1);
}

TEST_F(HistogramTemplatesTest, CutStageHistogramsSanitizesNames) {
    std::vector<std::string> cuts = {"All Events", "p > 2 GeV", "theta < 35"};
    CutStageHistograms h_momentum(m_hist_svc, "/Electron/cutflow/momentum",
                                   "Electron Momentum", 100, 0.0, 10.0, cuts);

    // All cuts should be accessible by their original names
    for (const auto& cut : cuts) {
        EXPECT_NE(h_momentum[cut], nullptr) << "Cut '" << cut << "' should be accessible";
    }
}

// ============================================================================
// HistogramRegistry Tests
// ============================================================================

TEST_F(HistogramTemplatesTest, HistogramRegistryDeclareAndBook) {
    HistogramRegistry registry;
    registry.set_service(&m_hist_svc);

    registry.declare("/Test/momentum", "Momentum;p [GeV/c];Events", 100, 0.0, 10.0);
    registry.declare("/Test/theta", "Theta;#theta [deg];Events", 180, 0.0, 180.0);

    EXPECT_EQ(registry.num_hist1d(), 2);
    EXPECT_TRUE(registry.book_all());

    // Access by name (extracted from path)
    registry["momentum"].fill(5.0);
    registry["theta"].fill(45.0);

    // Verify fills using atomic histograms directly
    EXPECT_EQ(m_hist_svc.get_hist1D("/Test/momentum")->entries(), 1);
    EXPECT_EQ(m_hist_svc.get_hist1D("/Test/theta")->entries(), 1);
}

TEST_F(HistogramTemplatesTest, HistogramRegistryWithoutService) {
    HistogramRegistry registry;
    // Don't set service

    registry.declare("/Test/momentum", "Momentum", 100, 0.0, 10.0);
    EXPECT_FALSE(registry.book_all());  // Should fail without service
}

// ============================================================================
// Automatic Persistence Tests
// ============================================================================

TEST_F(HistogramTemplatesTest, SectorHistogramsPersistence) {
    {
        // Create and fill histograms in a scope
        SectorHistograms<6> h_momentum(m_hist_svc, "/Persist/momentum",
                                        "Electron Momentum", 100, 0.0, 10.0);

        for (int s = 1; s <= 6; ++s) {
            h_momentum.fill(s, 5.0);
        }

        // Finalize writes histograms and close file
        m_hist_svc.finalize();
        m_output_svc.finalize();  // Close the file so we can read it
    }

    // Verify histograms were written to file
    TFile file(m_output_file.c_str(), "READ");
    ASSERT_FALSE(file.IsZombie());
    ASSERT_TRUE(file.IsOpen());

    for (int s = 1; s <= 6; ++s) {
        std::string name = fmt::format("momentum_S{}", s);
        // Navigate to directory and get histogram
        TDirectory* dir = file.GetDirectory("Persist");
        ASSERT_NE(dir, nullptr) << "Directory Persist not found";
        TH1D* hist = dynamic_cast<TH1D*>(dir->Get(name.c_str()));
        EXPECT_NE(hist, nullptr) << "Histogram " << name << " not found in Persist directory";
        if (hist) {
            EXPECT_EQ(hist->GetEntries(), 1) << "Histogram " << name << " should have 1 entry";
        }
    }

    file.Close();

    // Re-open output service for TearDown cleanup
    m_output_svc.set_output_file(m_output_file);
    m_output_svc.initialize();
    m_hist_svc.set_output_file_service(&m_output_svc);
}

TEST_F(HistogramTemplatesTest, CutStageHistogramsPersistence) {
    {
        std::vector<std::string> cuts = {"All", "Selected"};
        CutStageHistograms h_momentum(m_hist_svc, "/Persist/cutflow/momentum",
                                       "Electron Momentum", 100, 0.0, 10.0, cuts);

        h_momentum.fill_after_cut("All", 5.0);
        h_momentum.fill_after_cut("Selected", 5.0);

        // Finalize writes histograms and close file
        m_hist_svc.finalize();
        m_output_svc.finalize();  // Close the file so we can read it
    }

    // Verify histograms were written to file
    TFile file(m_output_file.c_str(), "READ");
    ASSERT_FALSE(file.IsZombie());
    ASSERT_TRUE(file.IsOpen());

    // Navigate to directory structure
    TDirectory* persist_dir = file.GetDirectory("Persist");
    ASSERT_NE(persist_dir, nullptr) << "Directory Persist not found";
    TDirectory* cutflow_dir = persist_dir->GetDirectory("cutflow");
    ASSERT_NE(cutflow_dir, nullptr) << "Directory cutflow not found";

    TH1D* h_all = dynamic_cast<TH1D*>(cutflow_dir->Get("momentum_All"));
    TH1D* h_selected = dynamic_cast<TH1D*>(cutflow_dir->Get("momentum_Selected"));

    EXPECT_NE(h_all, nullptr) << "All histogram not found";
    EXPECT_NE(h_selected, nullptr) << "Selected histogram not found";

    if (h_all) EXPECT_EQ(h_all->GetEntries(), 1);
    if (h_selected) EXPECT_EQ(h_selected->GetEntries(), 1);

    file.Close();

    // Re-open output service for TearDown cleanup
    m_output_svc.set_output_file(m_output_file);
    m_output_svc.initialize();
    m_hist_svc.set_output_file_service(&m_output_svc);
}

TEST_F(HistogramTemplatesTest, VerificationOption) {
    // Test that verification can be enabled/disabled
    m_hist_svc.set_verification(true);

    SectorHistograms<6> h_momentum(m_hist_svc, "/Verify/momentum",
                                    "Electron Momentum", 100, 0.0, 10.0);

    for (int s = 1; s <= 6; ++s) {
        h_momentum.fill(s, 5.0);
    }

    // Finalize should write histograms successfully
    auto sc = m_hist_svc.finalize();
    EXPECT_TRUE(sc.is_success());

    // Verify histograms exist in the service
    for (int s = 1; s <= 6; ++s) {
        std::string path = fmt::format("/Verify/momentum_S{}", s);
        EXPECT_NE(m_hist_svc.get_output_hist1D(path), nullptr) << "Histogram " << path << " should exist";
    }
}

// ============================================================================
// ThreadLocalHistogramGroup Tests
// ============================================================================

TEST_F(HistogramTemplatesTest, AtomicHistogramGroupCreation) {
    ThreadLocalHistogramGroup group(&m_hist_svc, "/Atomic");

    auto* h1 = group.add_hist1D("momentum", "Momentum;p [GeV/c];Events", 100, 0.0, 10.0);
    auto* h2 = group.add_hist1D("theta", "Theta;#theta [deg];Events", 180, 0.0, 180.0);

    EXPECT_NE(h1, nullptr);
    EXPECT_NE(h2, nullptr);
}

TEST_F(HistogramTemplatesTest, AtomicHistogramGroupFill) {
    ThreadLocalHistogramGroup group(&m_hist_svc, "/AtomicFill");

    group.add_hist1D("momentum", "Momentum", 100, 0.0, 10.0);
    group.add_hist2D("p_vs_theta", "p vs theta", 100, 0.0, 10.0, 180, 0.0, 180.0);

    group.fill("momentum", 5.0);
    group.fill2d("p_vs_theta", 5.0, 45.0);

    // Should not crash with non-existent histogram
    group.fill("nonexistent", 1.0);
}

TEST_F(HistogramTemplatesTest, AtomicHistogramGroupGetHistogram) {
    ThreadLocalHistogramGroup group(&m_hist_svc, "/AtomicGet");

    group.add_hist1D("momentum", "Momentum", 100, 0.0, 10.0);

    auto* h = group.hist1d("momentum");
    auto* h_missing = group.hist1d("nonexistent");

    EXPECT_NE(h, nullptr);
    EXPECT_EQ(h_missing, nullptr);
}

// ============================================================================
// SectorHistogramGroup Tests
// ============================================================================

TEST_F(HistogramTemplatesTest, SectorHistogramGroupBookingAndFill) {
    SectorHistogramGroup<6> g(&m_hist_svc, "/Tracks/Electrons");

    auto h_p  = g.add_hist1D("p", "p;p [GeV/c];Tracks", 100, 0, 10);
    auto h_pt = g.add_hist2D("p_vs_theta", "p vs theta;p [GeV/c];theta [deg]",
                             100, 0, 10, 90, 0, 45);

    // Fill only into sectors 1 and 3.
    h_p.fill(1, 5.0);
    h_p.fill(1, 5.5);
    h_p.fill(3, 3.0);

    h_pt.fill(1, 5.0, 20.0);
    h_pt.fill(3, 3.0, 15.0);

    // Out-of-range sector is a silent no-op (no crash, no exception).
    h_p.fill(0, 1.0);
    h_p.fill(7, 1.0);
    h_pt.fill(-1, 0.0, 0.0);

    // fill_if gates correctly.
    h_p.fill_if(false, 2, 9.9);
    EXPECT_EQ(h_p[2]->entries(), 0u);

    // Per-sector entry counts reflect only the sectors we filled.
    EXPECT_EQ(h_p[1]->entries(), 2u);
    EXPECT_EQ(h_p[2]->entries(), 0u);
    EXPECT_EQ(h_p[3]->entries(), 1u);
    EXPECT_EQ(h_p[4]->entries(), 0u);

    EXPECT_EQ(h_pt[1]->entries(), 1u);
    EXPECT_EQ(h_pt[3]->entries(), 1u);
    EXPECT_EQ(h_pt[4]->entries(), 0u);

    // Late-lookup returns the same handle.
    auto h_p_again = g.hist1d("p");
    EXPECT_EQ(h_p_again[1], h_p[1]);
    EXPECT_EQ(h_p_again[6], h_p[6]);

    auto missing = g.hist1d("does_not_exist");
    EXPECT_EQ(missing[1], nullptr);

    // After merge, each sector's TH1D lives at the expected path.
    m_hist_svc.merge_histograms();
    EXPECT_NE(m_hist_svc.get_output_hist1D("/Tracks/Electrons/p_S1"), nullptr);
    EXPECT_NE(m_hist_svc.get_output_hist1D("/Tracks/Electrons/p_S6"), nullptr);
    EXPECT_NE(m_hist_svc.get_output_hist2D("/Tracks/Electrons/p_vs_theta_S3"), nullptr);
}

TEST_F(HistogramTemplatesTest, Book3DHistogramAndRoundTrip) {
    auto* h = m_hist_svc.book_hist3D(
        "/Test/th3",
        "TH3 test;x;y;z",
        5, 0.0, 5.0,
        4, 0.0, 4.0,
        3, 0.0, 3.0);
    ASSERT_NE(h, nullptr);

    // Fill a few known bins.
    h->fill(0.5, 0.5, 0.5);
    h->fill(2.5, 1.5, 2.5);
    h->fill(2.5, 1.5, 2.5);

    // Pre-merge: no TH3D yet.
    EXPECT_EQ(m_hist_svc.get_output_hist3D("/Test/th3"), nullptr);

    m_hist_svc.merge_histograms();
    auto* th3 = m_hist_svc.get_output_hist3D("/Test/th3");
    ASSERT_NE(th3, nullptr);
    EXPECT_EQ(th3->GetEntries(), 3.0);
    EXPECT_DOUBLE_EQ(th3->GetBinContent(
        th3->GetXaxis()->FindBin(2.5),
        th3->GetYaxis()->FindBin(1.5),
        th3->GetZaxis()->FindBin(2.5)), 2.0);
}

TEST_F(HistogramTemplatesTest, SectorHistogramGroupNullService) {
    // Construction with a null service must not crash; handles are all null.
    SectorHistogramGroup<6> g(nullptr, "/X");
    auto h = g.add_hist1D("p", "p", 10, 0, 1);
    for (int s = 1; s <= 6; ++s) EXPECT_EQ(h[s], nullptr);
    // Fill into null slots is a silent no-op.
    h.fill(1, 0.5);
    h.fill(6, 0.5);
}

// ============================================================================
// ThreadLocalHistogramTemplates Tests
// ============================================================================

TEST_F(HistogramTemplatesTest, ElectronHistogramsTemplate) {
    auto group = ThreadLocalHistogramTemplates::ElectronHistograms::create(&m_hist_svc, "/Templates/Electron");

    EXPECT_NE(group, nullptr);

    // Should have standard electron histograms
    group->fill("p", 5.0);
    group->fill("theta", 25.0);
    group->fill("phi", 45.0);
}

TEST_F(HistogramTemplatesTest, HadronHistogramsTemplate) {
    auto group = ThreadLocalHistogramTemplates::HadronHistograms::create(&m_hist_svc, "/Templates/Hadron");

    EXPECT_NE(group, nullptr);

    group->fill("p", 2.0);
    group->fill("beta", 0.95);
}

TEST_F(HistogramTemplatesTest, DISKinematicsTemplate) {
    auto group = ThreadLocalHistogramTemplates::DISKinematics::create(&m_hist_svc, "/Templates/DIS");

    EXPECT_NE(group, nullptr);

    group->fill("Q2", 2.5);
    group->fill("W", 2.0);
    group->fill("xB", 0.3);
}

// ============================================================================
// CutflowHistograms Tests
// ============================================================================

struct TestContext {
    double momentum = 0.0;
    double theta = 0.0;
    int sector = 0;
};

TEST_F(HistogramTemplatesTest, CutflowHistogramsCreation) {
    CutflowHistograms<TestContext> analysis("Test Analysis");

    analysis
        .define_always("All Events")
        .define("Momentum > 1 GeV", [](const TestContext& ctx) { return ctx.momentum > 1.0; })
        .define("Theta < 35", [](const TestContext& ctx) { return ctx.theta < 35.0; });

    EXPECT_EQ(analysis.num_cuts(), 3);
}

TEST_F(HistogramTemplatesTest, CutflowHistogramsEvaluate) {
    CutflowHistograms<TestContext> analysis("Test Analysis");

    analysis
        .define_always("All Events")
        .define("Momentum > 1 GeV", [](const TestContext& ctx) { return ctx.momentum > 1.0; })
        .define("Theta < 35", [](const TestContext& ctx) { return ctx.theta < 35.0; });

    // Set up a filler
    analysis.set_filler([](ThreadLocalHistogramGroup* h, const TestContext& ctx) {
        h->fill("p", ctx.momentum);
        h->fill("theta", ctx.theta);
    });

    // Initialize histograms
    EXPECT_TRUE(analysis.initialize(&m_hist_svc, "/CutflowTest"));

    // Test passing context
    TestContext passing{2.0, 30.0, 1};
    EXPECT_TRUE(analysis.evaluate_and_fill(passing));

    // Test failing context (momentum too low)
    TestContext failing{0.5, 30.0, 1};
    EXPECT_FALSE(analysis.evaluate_and_fill(failing));
}

TEST_F(HistogramTemplatesTest, CutflowHistogramsApply) {
    CutflowHistograms<TestContext> analysis("Test Analysis");

    analysis
        .define_always("All Events")
        .define("Momentum > 1 GeV", [](const TestContext& ctx) { return ctx.momentum > 1.0; });

    analysis.initialize(&m_hist_svc, "/CutflowApply");

    TestContext passing{2.0, 30.0, 1};
    TestContext failing{0.5, 30.0, 1};

    EXPECT_TRUE(analysis.apply(passing).is_success());
    EXPECT_TRUE(analysis.apply(failing).is_filtered());
}
