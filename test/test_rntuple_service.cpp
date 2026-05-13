#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/Field.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>

#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleView.hxx>

using framework::NTupleService;
using framework::NTupleWriter;
using framework::OutputFileService;

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

// -----------------------------------------------------------------------------
// Per-test row structs. Each one exposes the `static constexpr ntuple_fields()`
// hook that NTupleWriter<T> consumes. Mirrors how real algorithms declare
// their own row types.
// -----------------------------------------------------------------------------

namespace {

struct ScalarRow {
    double e_p     = 0.0;
    double e_theta = 0.0;
    int    sector  = 0;

    static constexpr auto ntuple_fields() {
        using T = ScalarRow;
        return std::make_tuple(
            framework::field("e_p",     &T::e_p),
            framework::field("e_theta", &T::e_theta),
            framework::field("sector",  &T::sector));
    }
};

struct VectorRow {
    std::vector<int> hits;

    static constexpr auto ntuple_fields() {
        using T = VectorRow;
        return std::make_tuple(
            framework::field("hits", &T::hits));
    }
};

struct EventsRow {
    double Q2 = 0.0;

    static constexpr auto ntuple_fields() {
        using T = EventsRow;
        return std::make_tuple(framework::field("Q2", &T::Q2));
    }
};

struct TracksRow {
    int id = 0;

    static constexpr auto ntuple_fields() {
        using T = TracksRow;
        return std::make_tuple(framework::field("id", &T::id));
    }
};

}  // namespace

// Scalar fields: register, fill a handful of entries, close the
// service, then reopen the file via RNTupleReader and check every
// entry matches.
TEST_F(NTupleServiceTest, ScalarRoundTrip) {
    NTupleWriter<ScalarRow> writer(&m_ntuple_svc, "events");

    const std::vector<double> p_values     = {1.5, 2.0, 3.25};
    const std::vector<double> theta_values = {0.1, 0.2, 0.3};
    const std::vector<int>    sector_values = {1, 3, 6};

    for (std::size_t i = 0; i < p_values.size(); ++i) {
        writer.data().e_p     = p_values[i];
        writer.data().e_theta = theta_values[i];
        writer.data().sector  = sector_values[i];
        writer.fill();
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
    NTupleWriter<VectorRow> writer(&m_ntuple_svc, "events");

    writer.data().hits = {1, 2, 3};
    writer.fill();

    writer.data().hits = {};
    writer.fill();

    writer.data().hits = {42};
    writer.fill();

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
// schema and entry count. Two NTupleWriter<T> instances bound to
// different tree names talk to two different RNTupleParallelWriter
// instances inside the service.
TEST_F(NTupleServiceTest, MultipleNtuples) {
    NTupleWriter<EventsRow> events_writer(&m_ntuple_svc, "events");
    NTupleWriter<TracksRow> tracks_writer(&m_ntuple_svc, "tracks");

    events_writer.data().Q2 = 1.5;
    events_writer.fill();
    events_writer.data().Q2 = 2.5;
    events_writer.fill();

    tracks_writer.data().id = 7;
    tracks_writer.fill();

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
