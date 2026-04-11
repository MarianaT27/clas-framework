#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>

#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleView.hxx>

using clas12::framework::NTupleService;
using clas12::framework::OutputFileService;

// Test fixture: create a fresh NTupleService wired to a temp ROOT file.
// TearDown removes the file. The service is rebuilt per test so each
// case starts from a clean slate.
class NTupleServiceTest : public ::testing::Test {
   protected:
    void SetUp() override {
        m_output_file = "test_rntuple_service_output.root";
        if (std::filesystem::exists(m_output_file)) {
            std::filesystem::remove(m_output_file);
        }

        m_out_svc.set_output_file(m_output_file);
        ASSERT_TRUE(m_out_svc.initialize().is_success());
        m_ntuple_svc.set_output_file_service(&m_out_svc);
        ASSERT_TRUE(m_ntuple_svc.initialize().is_success());
    }

    void TearDown() override {
        // Explicit order: finalize NTupleService before OutputFileService so
        // the writer flushes into a still-open TFile.
        m_ntuple_svc.finalize();
        m_out_svc.finalize();
        if (std::filesystem::exists(m_output_file)) {
            std::filesystem::remove(m_output_file);
        }
    }

    OutputFileService m_out_svc;
    NTupleService     m_ntuple_svc;
    std::string       m_output_file;
};

// Scalar fields: register, fill a handful of entries, close the
// service, then reopen the file via RNTupleReader and check every
// entry matches.
TEST_F(NTupleServiceTest, ScalarRoundTrip) {
    double e_p     = 0.0;
    double e_theta = 0.0;
    int    sector  = 0;

    m_ntuple_svc.register_branch("events", "e_p",     &e_p);
    m_ntuple_svc.register_branch("events", "e_theta", &e_theta);
    m_ntuple_svc.register_branch("events", "sector",  &sector);

    const std::vector<double> p_values     = {1.5, 2.0, 3.25};
    const std::vector<double> theta_values = {0.1, 0.2, 0.3};
    const std::vector<int>    sector_values = {1, 3, 6};

    for (std::size_t i = 0; i < p_values.size(); ++i) {
        e_p     = p_values[i];
        e_theta = theta_values[i];
        sector  = sector_values[i];
        m_ntuple_svc.fill("events");
    }

    EXPECT_EQ(m_ntuple_svc.get_entries("events"), 3);

    // Finalize the writer so the on-disk file is readable.
    m_ntuple_svc.finalize();
    m_out_svc.finalize();

    auto reader = ROOT::RNTupleReader::Open("events", m_output_file);
    ASSERT_NE(reader, nullptr);
    ASSERT_EQ(reader->GetNEntries(), 3U);

    auto p_view     = reader->GetView<double>("e_p");
    auto theta_view = reader->GetView<double>("e_theta");
    auto sec_view   = reader->GetView<int>("sector");

    for (std::size_t i = 0; i < p_values.size(); ++i) {
        EXPECT_DOUBLE_EQ(p_view(i),     p_values[i]);
        EXPECT_DOUBLE_EQ(theta_view(i), theta_values[i]);
        EXPECT_EQ(sec_view(i),          sector_values[i]);
    }

    // Re-open OutputFileService for TearDown cleanup.
    m_out_svc.set_output_file(m_output_file);
    m_out_svc.initialize();
    m_ntuple_svc.set_output_file_service(&m_out_svc);
}

// Vector field: std::vector<int> -> an RNTuple variable-length column.
// Different lengths per entry verify the jagged layout.
TEST_F(NTupleServiceTest, VectorRoundTrip) {
    std::vector<int> hits;

    m_ntuple_svc.register_vector_branch("events", "hits", &hits);

    hits = {1, 2, 3};
    m_ntuple_svc.fill("events");

    hits = {};
    m_ntuple_svc.fill("events");

    hits = {42};
    m_ntuple_svc.fill("events");

    EXPECT_EQ(m_ntuple_svc.get_entries("events"), 3);

    m_ntuple_svc.finalize();
    m_out_svc.finalize();

    auto reader = ROOT::RNTupleReader::Open("events", m_output_file);
    ASSERT_NE(reader, nullptr);
    ASSERT_EQ(reader->GetNEntries(), 3U);

    auto view = reader->GetView<std::vector<int>>("hits");
    EXPECT_EQ(view(0).size(), 3U);
    EXPECT_EQ(view(1).size(), 0U);
    EXPECT_EQ(view(2).size(), 1U);
    if (view(0).size() == 3) {
        EXPECT_EQ(view(0)[0], 1);
        EXPECT_EQ(view(0)[1], 2);
        EXPECT_EQ(view(0)[2], 3);
    }
    if (view(2).size() == 1) {
        EXPECT_EQ(view(2)[0], 42);
    }

    m_out_svc.set_output_file(m_output_file);
    m_out_svc.initialize();
    m_ntuple_svc.set_output_file_service(&m_out_svc);
}

// Multiple ntuples in the same file: each has its own independent
// schema and entry count.
TEST_F(NTupleServiceTest, MultipleNtuples) {
    double event_q2 = 0.0;
    int    track_id = 0;

    m_ntuple_svc.register_branch("events", "Q2", &event_q2);
    m_ntuple_svc.register_branch("tracks", "id", &track_id);

    event_q2 = 1.5;
    m_ntuple_svc.fill("events");
    event_q2 = 2.5;
    m_ntuple_svc.fill("events");

    track_id = 7;
    m_ntuple_svc.fill("tracks");

    EXPECT_EQ(m_ntuple_svc.get_entries("events"), 2);
    EXPECT_EQ(m_ntuple_svc.get_entries("tracks"), 1);

    m_ntuple_svc.finalize();
    m_out_svc.finalize();

    auto events = ROOT::RNTupleReader::Open("events", m_output_file);
    auto tracks = ROOT::RNTupleReader::Open("tracks", m_output_file);
    ASSERT_NE(events, nullptr);
    ASSERT_NE(tracks, nullptr);
    EXPECT_EQ(events->GetNEntries(), 2U);
    EXPECT_EQ(tracks->GetNEntries(), 1U);

    m_out_svc.set_output_file(m_output_file);
    m_out_svc.initialize();
    m_ntuple_svc.set_output_file_service(&m_out_svc);
}
