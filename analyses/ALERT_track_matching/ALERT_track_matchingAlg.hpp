#pragma once

/**
 * @file ElasticEPAlg.hpp
 * @brief Algorithm for Elastic e-p Scattering analysis
 *
 */

#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/core/ServiceHelpers.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/Particle.hpp>
#include <atomic>
#include <cmath>
#include <set>
#include <vector>
#include "Math/GenVector/PxPyPzE4D.h"
#include "Math/Vector3Dfwd.h"
#include "bank.h"

namespace ALERT_track_matching {

/**
 * @brief Algorithm for ALERT track matching analysis
 *
 * This algorithm demonstrates the SimpleAlgorithm CRTP pattern.
 * Customize the process() method to implement your analysis logic.
 */
class ALERT_track_matchingAlg : public clas12::framework::SimpleAlgorithm<ALERT_track_matchingAlg> {
public:
    /**
     * @brief Construct the algorithm
     * @param name Algorithm name (defaults to class name)
     */
    explicit ALERT_track_matchingAlg(std::string name = "ALERT_track_matchingAlg")
        : SimpleAlgorithm(std::move(name)) {
        // Declare configurable properties
        declare_property(m_example_cut);
        declare_property(m_enable_histograms);
        declare_property(m_phi_cut);
    }

    /**
     * @brief Configure service dependencies
     *
     * Called during initialization to declare required services.
     */
    auto configure() -> void override {
        // Declare required services
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
    }

    /**
     * @brief Custom initialization
     *
     * Called after services are injected. Book histograms and setup resources here.
     * @return StatusCode SUCCESS to continue, FAILURE to abort
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        if (m_enable_histograms && m_hist_svc && m_hist_svc->is_valid()) {
            // Book your histograms here
            // m_hist_svc->get()->book_hist1D("hist_name", "Title", 100, 0.0, 10.0);
            book_histograms();
        }

        // Read the file analyses/ALERT_track_matching/atof_wedge_H2.data
        // Read the ATOF wedge H2 data file
            std::ifstream atof_file("analyses/ALERT_track_matching/atof_wedge_H2.data");
            if (!atof_file.is_open()) {
                clas12::framework::Log::error(name(), "Failed to open ATOF wedge H2 data file");
                return clas12::framework::StatusCode::FAILURE();
            }
            std::string line;
            while (std::getline(atof_file, line)) {
                std::istringstream iss(line);
                ATOFWedgeH2Data data;
                iss >> data.sector >> data.layer >> data.component >> data.order
                    >> data.mean >> data.sigma >> data.a >> data.b >> data.c >> data.d;
                m_atof_wedge_H2_data.push_back(data);
            }
            atof_file.close();

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     *
     * Implement your per-event analysis logic here.
     * @param event The current event data
     * @return StatusCode SUCCESS to continue, FILTERED to skip, FAILURE to stop
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {

        // Check for elastic electron decision from upstream algorithm
        if (!event.passes_decision("HasElasticElectron")) {
            return clas12::framework::StatusCode::FILTERED();
        }
        // Get elastic electron using particle retrieval
        const auto* electrons = event.get_particles<clas12::Particle>("ElasticElectron");
        if (!electrons || electrons->empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        const auto& elastic_electron = electrons->front();


        // Read the AHDC::track, ALERT::ai:projections and ATOF::hits banks
        hipo::bank& ahdc_track_bank = event.hipo_event().get("AHDC::track");
        hipo::bank& alert_projections_bank = event.hipo_event().get("ALERT::ai:projections");
        hipo::bank& atof_hits_bank = event.hipo_event().get("ATOF::hits");

        // Select good atof hits based on the m_atof_wedge_H2_data
        std::vector<int> good_atof_hit_indices;
        for (int i = 0; i < atof_hits_bank.rows(); ++i) {
            int sector = atof_hits_bank.get<int>("sector", i);
            int layer = atof_hits_bank.get<int>("layer", i);
            int component = atof_hits_bank.get<int>("component", i);
            double time = atof_hits_bank.get<double>("time", i);
            // Find matching H2 data
            auto it = std::find_if(m_atof_wedge_H2_data.begin(), m_atof_wedge_H2_data.end(),
                                   [sector, layer, component](const ATOFWedgeH2Data& data) {
                                       return data.sector == sector && data.layer == layer && data.component == component;
                                   });
            if (it != m_atof_wedge_H2_data.end()) {
                // Time should be within mean +/- 3*sigma
                if (std::abs(time - it->mean) <= 3 * it->sigma) good_atof_hit_indices.push_back(i);
            }
        }

        // Read AHDC tracks
        std::vector<AHDCTrackData> good_tracks;
        for (int i = 0; i < ahdc_track_bank.rows(); ++i) {
            AHDCTrackData track;
            track.trackid = ahdc_track_bank.get<int>("trackid", i);
            track.x = ahdc_track_bank.get<float>("x", i);
            track.y = ahdc_track_bank.get<float>("y", i);
            track.z = ahdc_track_bank.get<float>("z", i);
            track.px = ahdc_track_bank.get<float>("px", i);
            track.py = ahdc_track_bank.get<float>("py", i);
            track.pz = ahdc_track_bank.get<float>("pz", i);
            track.n_hits = ahdc_track_bank.get<int>("n_hits", i);
            track.sum_adc = ahdc_track_bank.get<int>("sum_adc", i);
            track.path = ahdc_track_bank.get<float>("path", i);
            track.dEdx = ahdc_track_bank.get<float>("dEdx", i);
            track.p_drift = ahdc_track_bank.get<float>("p_drift", i);
            track.chi2 = ahdc_track_bank.get<float>("chi2", i);
            track.sum_residuals = ahdc_track_bank.get<float>("sum_residuals", i);
            good_tracks.push_back(track);
        }

        // Fill diagnostic histograms
        if (h_n_predictions) h_n_predictions->fill(good_tracks.size());
        if (h_n_good_atof_hits) h_n_good_atof_hits->fill(good_atof_hit_indices.size());

        if (good_tracks.empty() || good_atof_hit_indices.empty()) {
            clas12::framework::Log::info(name(), "No good tracks or ATOF hits found in this event");
            return clas12::framework::StatusCode::FILTERED();
        }

        // Match tracks to the elastic electron using phi angle
        // For elastic ep, proton should be back-to-back with electron (150-200° apart)
        double electron_phi = elastic_electron.phi();  // degrees

        // Select tracks where delta_phi is between 150 and 200 degrees
        std::vector<const AHDCTrackData*> matched_tracks;

        for (const auto& track : good_tracks) {
            // Compute track phi from momentum
            double track_phi = std::atan2(track.py, track.px) * 180.0 / M_PI;

            // Compute phi difference (0-360 range)
            double delta_phi = std::abs(electron_phi - track_phi);
            if (delta_phi > 180.0) delta_phi = 360.0 - delta_phi;

            // Fill histogram for all tracks
            if (h_delta_phi) h_delta_phi->fill(delta_phi);

            // Select tracks with delta_phi between 150 and 200 degrees
            if (delta_phi >= 150.0 && delta_phi <= 200.0) {
                matched_tracks.push_back(&track);
                if (h_best_delta_phi) h_best_delta_phi->fill(delta_phi);
            }
        }

        if (matched_tracks.empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Process all matched tracks
        for (const auto* matched_track : matched_tracks) {
            // Count: track matched to electron (denominator for efficiency)
            ++m_n_tracks_matched_to_electron;

            // Find the ATOF hit associated with the matched track using ALERT::ai:projections
            int matched_atof_hit_id = -1;
            for (int i = 0; i < alert_projections_bank.rows(); ++i) {
                int proj_trackid = alert_projections_bank.get<int>("trackid", i);
                if (proj_trackid == matched_track->trackid) {
                    matched_atof_hit_id = alert_projections_bank.get<int>("matched_atof_hit_id", i);
                    break;
                }
            }

            // Fill matched ATOF hit ID histogram
            if (h_matched_atof_hit_id) h_matched_atof_hit_id->fill(matched_atof_hit_id);

            // Collect sectors/layers/components of good ATOF hits
            std::set<int> good_sectors, good_layers, good_components;
            for (int idx : good_atof_hit_indices) {
                good_sectors.insert(atof_hits_bank.get<int>("sector", idx));
                good_layers.insert(atof_hits_bank.get<int>("layer", idx));
                good_components.insert(atof_hits_bank.get<int>("component", idx));
            }

            // Determine AI match status and count
            int match_status = 0;  // 0 = no match
            if (matched_atof_hit_id >= 0 && matched_atof_hit_id < atof_hits_bank.rows()) {
                ++m_n_tracks_with_ai_atof_match;
                match_status = 1;  // 1 = AI found a match

                // Get sector/layer/component of the AI-matched ATOF hit
                int ai_sector = atof_hits_bank.get<int>("sector", matched_atof_hit_id);
                int ai_layer = atof_hits_bank.get<int>("layer", matched_atof_hit_id);
                int ai_component = atof_hits_bank.get<int>("component", matched_atof_hit_id);

                // Check if AI matched to correct sector/layer/component (matches any good hit)
                if (good_sectors.count(ai_sector) > 0) {
                    ++m_n_correct_sector;
                }
                if (good_layers.count(ai_layer) > 0) {
                    ++m_n_correct_layer;
                }
                if (good_components.count(ai_component) > 0) {
                    ++m_n_correct_component;
                }
                // Check if AI component is within ±1 of a good component (correct or adjacent)
                if (good_components.count(ai_component) > 0 ||
                    good_components.count(ai_component - 1) > 0 ||
                    good_components.count(ai_component + 1) > 0) {
                    ++m_n_component_off_by_one;
                }

                // Check if AI match is a good (quality-cut) ATOF hit
                bool atof_hit_is_good = std::find(good_atof_hit_indices.begin(), good_atof_hit_indices.end(),
                                                  matched_atof_hit_id) != good_atof_hit_indices.end();
                if (atof_hit_is_good) {
                    ++m_n_tracks_with_good_atof_match;
                    match_status = 2;  // 2 = good match
                }
            }

            // Fill AI match status histogram
            if (h_ai_match_status) h_ai_match_status->fill(match_status);
        }  // end for matched_tracks

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Custom finalization
     *
     * Called after all events. Print statistics and cleanup here.
     * @return StatusCode SUCCESS or FAILURE
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        // Print AI ATOF matching efficiency
        using Log = clas12::framework::Log;

        int n_matched = m_n_tracks_matched_to_electron.load();
        int n_ai_match = m_n_tracks_with_ai_atof_match.load();
        int n_good_match = m_n_tracks_with_good_atof_match.load();

        int n_correct_sector = m_n_correct_sector.load();
        int n_correct_layer = m_n_correct_layer.load();
        int n_correct_component = m_n_correct_component.load();
        int n_component_off_by_one = m_n_component_off_by_one.load();

        Log::info(name(), "=== AI Track-ATOF Matching Efficiency ===");
        Log::info(name(), "Tracks matched to electron (by phi): {}", n_matched);
        Log::info(name(), "Tracks with AI ATOF match:           {} ({:.1f}%)",
                  n_ai_match, n_matched > 0 ? 100.0 * n_ai_match / n_matched : 0.0);
        Log::info(name(), "Tracks with good AI ATOF match:      {} ({:.1f}%)",
                  n_good_match, n_matched > 0 ? 100.0 * n_good_match / n_matched : 0.0);

        Log::info(name(), "");
        Log::info(name(), "=== Sector/Layer/Component Efficiency ===");
        Log::info(name(), "(AI matches correct value vs total AI matches)");
        Log::info(name(), "Sector efficiency:         {:5d} / {:5d} = {:5.1f}%",
                  n_correct_sector, n_ai_match, n_ai_match > 0 ? 100.0 * n_correct_sector / n_ai_match : 0.0);
        Log::info(name(), "Layer efficiency:          {:5d} / {:5d} = {:5.1f}%",
                  n_correct_layer, n_ai_match, n_ai_match > 0 ? 100.0 * n_correct_layer / n_ai_match : 0.0);
        Log::info(name(), "Component efficiency:      {:5d} / {:5d} = {:5.1f}%",
                  n_correct_component, n_ai_match, n_ai_match > 0 ? 100.0 * n_correct_component / n_ai_match : 0.0);
        Log::info(name(), "Component (within +-1):    {:5d} / {:5d} = {:5.1f}%",
                  n_component_off_by_one, n_ai_match, n_ai_match > 0 ? 100.0 * n_component_off_by_one / n_ai_match : 0.0);

        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    // Configurable properties
    clas12::framework::Property<double> m_example_cut{
        "example_cut", 1.0, "Example cut threshold value"
    };
    clas12::framework::Property<bool> m_enable_histograms{
        "enable_histograms", true, "Enable histogram filling"
    };
    clas12::framework::Property<double> m_phi_cut{
        "phi_cut", 10.0, "Phi matching tolerance in degrees"
    };

    // Service handles
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;

    // Lock-free atomic histograms (managed by HistogramService)
    clas12::framework::AtomicHistogram1D* h_p = nullptr;
    clas12::framework::AtomicHistogram1D* h_phi = nullptr;
    clas12::framework::AtomicHistogram1D* h_theta = nullptr;
    clas12::framework::AtomicHistogram1D* h_vz = nullptr;
    clas12::framework::AtomicHistogram1D* h_vz_Cu = nullptr;
    clas12::framework::AtomicHistogram1D* h_vz_Sn = nullptr;
    clas12::framework::AtomicHistogram1D* h_delta_phi = nullptr;
    clas12::framework::AtomicHistogram1D* h_n_predictions = nullptr;
    clas12::framework::AtomicHistogram1D* h_ai_match_status = nullptr;
    clas12::framework::AtomicHistogram1D* h_best_delta_phi = nullptr;
    clas12::framework::AtomicHistogram1D* h_n_good_atof_hits = nullptr;
    clas12::framework::AtomicHistogram1D* h_matched_atof_hit_id = nullptr;

    // Efficiency histograms by sector/layer/component (denominator = all AI matches, numerator = good matches)
    clas12::framework::AtomicHistogram1D* h_eff_sector_denom = nullptr;
    clas12::framework::AtomicHistogram1D* h_eff_sector_numer = nullptr;
    clas12::framework::AtomicHistogram1D* h_eff_layer_denom = nullptr;
    clas12::framework::AtomicHistogram1D* h_eff_layer_numer = nullptr;
    clas12::framework::AtomicHistogram1D* h_eff_component_denom = nullptr;
    clas12::framework::AtomicHistogram1D* h_eff_component_numer = nullptr;

    auto book_histograms() -> void {
        clas12::framework::safe(m_hist_svc).with([this](auto* svc) {
            // Use lock-free atomic histograms for parallel accumulation
            h_p = svc->book_atomic_hist1D("/MC/Electron/p", "p;p;Events", 100, 0, 10);
            h_phi = svc->book_atomic_hist1D("/MC/Electron/phi", "#phi;#phi;Events", 100, -180, 180);
            h_theta = svc->book_atomic_hist1D("/MC/Electron/theta", "#theta;#theta;Events", 100, 0, 90);
            h_vz = svc->book_atomic_hist1D("/MC/Electron/vz", "vz;vz;Events", 100, -30, 30);
            h_vz_Cu = svc->book_atomic_hist1D("/MC/Electron/vz_Cu", "vz_Cu;vz_Cu;Events", 100, -30, 30);
            h_vz_Sn = svc->book_atomic_hist1D("/MC/Electron/vz_Sn", "vz_Sn;vz_Sn;Events", 100, -30, 30);
            h_delta_phi = svc->book_atomic_hist1D("/ALERT/TrackMatching/delta_phi", "#Delta#phi;#Delta#phi [deg];Events", 360, 0, 360);
            h_n_predictions = svc->book_atomic_hist1D("/ALERT/TrackMatching/n_predictions", "N tracks;N tracks;Events", 20, 0, 20);
            h_ai_match_status = svc->book_atomic_hist1D("/ALERT/TrackMatching/ai_match_status", "AI Match Status;Status (0=none, 1=any, 2=good);Events", 3, 0, 3);
            h_best_delta_phi = svc->book_atomic_hist1D("/ALERT/TrackMatching/best_delta_phi", "Best #Delta#phi;#Delta#phi [deg];Events", 90, 0, 90);
            h_n_good_atof_hits = svc->book_atomic_hist1D("/ALERT/TrackMatching/n_good_atof_hits", "N good ATOF hits;N hits;Events", 20, 0, 20);
            h_matched_atof_hit_id = svc->book_atomic_hist1D("/ALERT/TrackMatching/matched_atof_hit_id", "Matched ATOF hit ID;Hit ID;Events", 50, -1, 49);

            // Efficiency histograms by sector/layer/component
            h_eff_sector_denom = svc->book_atomic_hist1D("/ALERT/Efficiency/sector_denom", "Sector (all AI matches);Sector;Events", 16, 0, 16);
            h_eff_sector_numer = svc->book_atomic_hist1D("/ALERT/Efficiency/sector_numer", "Sector (good matches);Sector;Events", 16, 0, 16);
            h_eff_layer_denom = svc->book_atomic_hist1D("/ALERT/Efficiency/layer_denom", "Layer (all AI matches);Layer;Events", 4, 0, 4);
            h_eff_layer_numer = svc->book_atomic_hist1D("/ALERT/Efficiency/layer_numer", "Layer (good matches);Layer;Events", 4, 0, 4);
            h_eff_component_denom = svc->book_atomic_hist1D("/ALERT/Efficiency/component_denom", "Component (all AI matches);Component;Events", 10, 0, 10);
            h_eff_component_numer = svc->book_atomic_hist1D("/ALERT/Efficiency/component_numer", "Component (good matches);Component;Events", 10, 0, 10);
        });
    }

    struct ATOFWedgeH2Data {
        int sector, layer, component, order;
        double mean, sigma, a, b, c, d; 
    };

    std::vector<ATOFWedgeH2Data> m_atof_wedge_H2_data;

    // Efficiency counters (atomic for thread safety)
    std::atomic<int> m_n_tracks_matched_to_electron{0};   // denominator
    std::atomic<int> m_n_tracks_with_ai_atof_match{0};    // AI found any ATOF match
    std::atomic<int> m_n_tracks_with_good_atof_match{0};  // AI match is a good hit
    std::atomic<int> m_n_correct_sector{0};               // AI matched correct sector
    std::atomic<int> m_n_correct_layer{0};                // AI matched correct layer
    std::atomic<int> m_n_correct_component{0};            // AI matched correct component
    std::atomic<int> m_n_component_off_by_one{0};         // AI matched correct or adjacent component (within ±1)

    struct AHDCTrackData {
        int trackid;
        float x, y, z;           // position in mm
        float px, py, pz;        // momentum in MeV
        int n_hits;
        int sum_adc;
        float path;              // track length in mm
        float dEdx;              // energy loss in adc/mm
        float p_drift;           // momentum in drift region in MeV
        float chi2;
        float sum_residuals;
    };
};

}  // namespace myanalysis
