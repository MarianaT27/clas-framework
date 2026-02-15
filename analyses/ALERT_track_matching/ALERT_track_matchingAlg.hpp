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
        declare_property(m_ai_hit_id_offset);
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
        double electron_phi = elastic_electron.phi();  // degrees

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
            if (component >= 10) continue;
            if (it != m_atof_wedge_H2_data.end()) {
                // Time should be within mean +/- 3*sigma
                if (std::abs(time - it->mean) <= 3 * it->sigma) good_atof_hit_indices.push_back(i);
            }
        }

        // Compute delta_phi between electron and each good ATOF hit, select "true" hits
        std::vector<int> true_atof_hit_indices;
        std::vector<double> true_atof_hit_phis;

        for (int idx : good_atof_hit_indices) {
            float hit_x = atof_hits_bank.get<float>("x", idx);
            float hit_y = atof_hits_bank.get<float>("y", idx);
            double atof_phi = std::atan2(hit_y, hit_x) * 180.0 / M_PI;

            double delta_phi = std::fmod(electron_phi - atof_phi, 360.0);
            if (delta_phi < 0.0) delta_phi += 360.0;

            if (h_atof_electron_delta_phi_all) h_atof_electron_delta_phi_all->fill(delta_phi);

            if (delta_phi >= 150.0 && delta_phi <= 200.0) {
                true_atof_hit_indices.push_back(idx);
                true_atof_hit_phis.push_back(atof_phi);
                if (h_atof_electron_delta_phi_true) h_atof_electron_delta_phi_true->fill(delta_phi);
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
        if (h_n_true_atof_hits) h_n_true_atof_hits->fill(true_atof_hit_indices.size());

        if (good_tracks.empty() || good_atof_hit_indices.empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Match tracks to the elastic electron using phi angle
        // For elastic ep, proton should be back-to-back with electron (150-200° apart)
        // Select tracks where delta_phi is between 150 and 200 degrees
        std::vector<const AHDCTrackData*> matched_tracks;

        for (const auto& track : good_tracks) {
            // Compute track phi from momentum
            double track_phi = std::atan2(track.py, track.px) * 180.0 / M_PI;

            double delta_phi = std::fmod(electron_phi - track_phi, 360.0);
            if (delta_phi < 0.0) delta_phi += 360.0;

            // Fill histogram for all tracks
            if (h_delta_phi) h_delta_phi->fill(delta_phi);

            // Select tracks with delta_phi between 150 and 200 degrees
            if (delta_phi >= 150.0 && delta_phi <= 200.0) {
                matched_tracks.push_back(&track);
                if (h_best_delta_phi) h_best_delta_phi->fill(delta_phi);
            }
        }

        if (matched_tracks.empty() || matched_tracks.size() > 1) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Select best true ATOF hit: closest in phi to the matched track prediction
        int best_true_atof_hit_index = -1;
        double best_phi_distance = 999.0;

        if (!true_atof_hit_indices.empty()) {
            double track_phi = std::atan2(matched_tracks.front()->py, matched_tracks.front()->px) * 180.0 / M_PI;

            for (size_t i = 0; i < true_atof_hit_indices.size(); ++i) {
                double dphi = std::fmod(track_phi - true_atof_hit_phis[i], 360.0);
                if (dphi < 0.0) dphi += 360.0;
                if (dphi > 180.0) dphi = 360.0 - dphi;

                if (h_track_atof_dphi) h_track_atof_dphi->fill(dphi);

                if (dphi < best_phi_distance && dphi <= m_phi_cut) {
                    best_phi_distance = dphi;
                    best_true_atof_hit_index = true_atof_hit_indices[i];
                }
            }

            if (best_true_atof_hit_index >= 0) {
                float hx = atof_hits_bank.get<float>("x", best_true_atof_hit_index);
                float hy = atof_hits_bank.get<float>("y", best_true_atof_hit_index);
                double best_atof_phi = std::atan2(hy, hx) * 180.0 / M_PI;
                double best_delta_phi = std::fmod(electron_phi - best_atof_phi, 360.0);
                if (best_delta_phi < 0.0) best_delta_phi += 360.0;
                if (h_atof_electron_delta_phi_best) h_atof_electron_delta_phi_best->fill(best_delta_phi);
                if (h_best_track_atof_dphi) h_best_track_atof_dphi->fill(best_phi_distance);
                if (h_track_phi_vs_atof_phi) h_track_phi_vs_atof_phi->fill(best_atof_phi, track_phi);
            }
        }

        // Process the single matched track
        for (const auto* matched_track : matched_tracks) {
            ++m_n_tracks_matched_to_electron;

            // Find the ATOF hit associated with the matched track using ALERT::ai:projections
            int matched_atof_hit_id = -1;
            for (int i = 0; i < alert_projections_bank.rows(); ++i) {
                int proj_trackid = alert_projections_bank.get<int>("trackid", i);
                if (proj_trackid == matched_track->trackid) {
                    matched_atof_hit_id = alert_projections_bank.get<int>("matched_atof_hit_id", i) - m_ai_hit_id_offset;
                    break;
                }
            }

            if (h_matched_atof_hit_id) h_matched_atof_hit_id->fill(matched_atof_hit_id);

            // Efficiency: only for the best true ATOF hit
            if (best_true_atof_hit_index >= 0) {
                ++m_n_true_sample;  // denominator: true track + true ATOF hit

                if (matched_atof_hit_id >= 0 && matched_atof_hit_id < atof_hits_bank.rows()) {
                    ++m_n_ai_has_match;

                    int ai_sector = atof_hits_bank.get<int>("sector", matched_atof_hit_id);
                    int ai_layer = atof_hits_bank.get<int>("layer", matched_atof_hit_id);
                    int ai_component = atof_hits_bank.get<int>("component", matched_atof_hit_id);

                    int true_sector = atof_hits_bank.get<int>("sector", best_true_atof_hit_index);
                    int true_layer = atof_hits_bank.get<int>("layer", best_true_atof_hit_index);
                    int true_component = atof_hits_bank.get<int>("component", best_true_atof_hit_index);

                    if (h_delta_component) h_delta_component->fill(ai_component - true_component);
                    if (h_delta_sector) h_delta_sector->fill(ai_sector - true_sector);
                    if (h_ai_vs_true_component) h_ai_vs_true_component->fill(true_component, ai_component);
                    if (h_ai_vs_true_layer) h_ai_vs_true_layer->fill(true_layer, ai_layer);
                    if (h_ai_vs_true_sector) h_ai_vs_true_sector->fill(true_sector, ai_sector);

                    if (ai_sector == true_sector) ++m_n_correct_sector;
                    if (ai_layer == true_layer) ++m_n_correct_layer;
                    if (ai_component == true_component) ++m_n_correct_component;
                    if (std::abs(ai_component - true_component) <= 1) ++m_n_component_off_by_one;
                    if (ai_sector == true_sector && ai_layer == true_layer) ++m_n_correct_sector_layer;
                    if (ai_sector == true_sector && ai_layer == true_layer && ai_component == true_component) ++m_n_correct_wedge;
                    if (ai_sector == true_sector && ai_layer == true_layer && std::abs(ai_component - true_component) <= 1) ++m_n_wedge_off_by_one;

                    if (matched_atof_hit_id == best_true_atof_hit_index) {
                        ++m_n_tracks_with_true_atof_match;  // numerator: AI found the best true hit
                    }
                }
            }
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
        int n_true_sample = m_n_true_sample.load();
        int n_ai_has_match = m_n_ai_has_match.load();
        int n_true_match = m_n_tracks_with_true_atof_match.load();
        int n_correct_sector = m_n_correct_sector.load();
        int n_correct_layer = m_n_correct_layer.load();
        int n_correct_component = m_n_correct_component.load();
        int n_component_off_by_one = m_n_component_off_by_one.load();
        int n_correct_sector_layer = m_n_correct_sector_layer.load();
        int n_correct_wedge = m_n_correct_wedge.load();
        int n_wedge_off_by_one = m_n_wedge_off_by_one.load();

        Log::info(name(), "=== AI Track-ATOF Matching Efficiency ===");
        Log::info(name(), "Tracks matched to electron (by dphi): {}", n_matched);
        Log::info(name(), "True sample (true track + true ATOF): {}", n_true_sample);
        Log::info(name(), "AI returned a valid ATOF match:       {:5d} / {:5d} = {:5.1f}%",
                  n_ai_has_match, n_true_sample, n_true_sample > 0 ? 100.0 * n_ai_has_match / n_true_sample : 0.0);
        Log::info(name(), "AI match = best true ATOF hit:        {:5d} / {:5d} = {:5.1f}%",
                  n_true_match, n_true_sample, n_true_sample > 0 ? 100.0 * n_true_match / n_true_sample : 0.0);
        Log::info(name(), "--- Of events where AI returned a match ---");
        Log::info(name(), "Sector efficiency:                    {:5d} / {:5d} = {:5.1f}%",
                  n_correct_sector, n_ai_has_match, n_ai_has_match > 0 ? 100.0 * n_correct_sector / n_ai_has_match : 0.0);
        Log::info(name(), "Layer efficiency:                     {:5d} / {:5d} = {:5.1f}%",
                  n_correct_layer, n_ai_has_match, n_ai_has_match > 0 ? 100.0 * n_correct_layer / n_ai_has_match : 0.0);
        Log::info(name(), "Component efficiency:                 {:5d} / {:5d} = {:5.1f}%",
                  n_correct_component, n_ai_has_match, n_ai_has_match > 0 ? 100.0 * n_correct_component / n_ai_has_match : 0.0);
        Log::info(name(), "Component (within +-1):               {:5d} / {:5d} = {:5.1f}%",
                  n_component_off_by_one, n_ai_has_match, n_ai_has_match > 0 ? 100.0 * n_component_off_by_one / n_ai_has_match : 0.0);
        Log::info(name(), "Sector+Layer:                         {:5d} / {:5d} = {:5.1f}%",
                  n_correct_sector_layer, n_ai_has_match, n_ai_has_match > 0 ? 100.0 * n_correct_sector_layer / n_ai_has_match : 0.0);
        Log::info(name(), "Wedge (sector+layer+component):       {:5d} / {:5d} = {:5.1f}%",
                  n_correct_wedge, n_ai_has_match, n_ai_has_match > 0 ? 100.0 * n_correct_wedge / n_ai_has_match : 0.0);
        Log::info(name(), "Wedge (component +-1):                {:5d} / {:5d} = {:5.1f}%",
                  n_wedge_off_by_one, n_ai_has_match, n_ai_has_match > 0 ? 100.0 * n_wedge_off_by_one / n_ai_has_match : 0.0);
        Log::info(name(), "Exact hit match:                      {:5d} / {:5d} = {:5.1f}%",
                  n_true_match, n_ai_has_match, n_ai_has_match > 0 ? 100.0 * n_true_match / n_ai_has_match : 0.0);
        Log::info(name(), "--- Overall efficiency ---");
        Log::info(name(), "AI found best ATOF hit (over all):    {:5d} / {:5d} = {:5.1f}%",
                  n_true_match, n_matched, n_matched > 0 ? 100.0 * n_true_match / n_matched : 0.0);

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
        "phi_cut", 30.0, "Phi matching tolerance in degrees"
    };
    clas12::framework::Property<int> m_ai_hit_id_offset{
        "ai_hit_id_offset", 0, "Offset to subtract from AI matched_atof_hit_id (0 if 0-based, 1 if 1-based)"
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
    clas12::framework::AtomicHistogram1D* h_best_delta_phi = nullptr;
    clas12::framework::AtomicHistogram1D* h_n_good_atof_hits = nullptr;
    clas12::framework::AtomicHistogram1D* h_matched_atof_hit_id = nullptr;

    // ATOF-electron delta_phi histograms
    clas12::framework::AtomicHistogram1D* h_atof_electron_delta_phi_all = nullptr;   // all good ATOF hits
    clas12::framework::AtomicHistogram1D* h_atof_electron_delta_phi_true = nullptr;  // true ATOF hits (passing delta_phi cut)
    clas12::framework::AtomicHistogram1D* h_atof_electron_delta_phi_best = nullptr;  // best true ATOF hit
    clas12::framework::AtomicHistogram1D* h_n_true_atof_hits = nullptr;

    // Debug histograms
    clas12::framework::AtomicHistogram1D* h_track_atof_dphi = nullptr;          // track-ATOF phi distance for all true hits
    clas12::framework::AtomicHistogram1D* h_best_track_atof_dphi = nullptr;     // best track-ATOF phi distance (selected truth)
    clas12::framework::AtomicHistogram1D* h_delta_component = nullptr;          // AI component - true component
    clas12::framework::AtomicHistogram1D* h_delta_sector = nullptr;             // AI sector - true sector
    clas12::framework::AtomicHistogram2D* h_ai_vs_true_component = nullptr;     // AI component vs true component
    clas12::framework::AtomicHistogram2D* h_ai_vs_true_layer = nullptr;         // AI layer vs true layer
    clas12::framework::AtomicHistogram2D* h_ai_vs_true_sector = nullptr;        // AI sector vs true sector
    clas12::framework::AtomicHistogram2D* h_track_phi_vs_atof_phi = nullptr;    // track phi vs best true ATOF hit phi

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
            h_best_delta_phi = svc->book_atomic_hist1D("/ALERT/TrackMatching/best_delta_phi", "Best #Delta#phi (track-electron);#Delta#phi [deg];Events", 100, 150, 200);
            h_n_good_atof_hits = svc->book_atomic_hist1D("/ALERT/TrackMatching/n_good_atof_hits", "N good ATOF hits;N hits;Events", 20, 0, 20);
            h_matched_atof_hit_id = svc->book_atomic_hist1D("/ALERT/TrackMatching/matched_atof_hit_id", "Matched ATOF hit ID;Hit ID;Events", 50, -1, 49);

            // ATOF-electron delta_phi histograms
            h_atof_electron_delta_phi_all = svc->book_atomic_hist1D("/ALERT/TrackMatching/atof_electron_delta_phi_all", "#Delta#phi(e, ATOF hit) all good;#Delta#phi [deg];Hits", 360, 0, 360);
            h_atof_electron_delta_phi_true = svc->book_atomic_hist1D("/ALERT/TrackMatching/atof_electron_delta_phi_true", "#Delta#phi(e, ATOF hit) true;#Delta#phi [deg];Hits", 100, 150, 200);
            h_atof_electron_delta_phi_best = svc->book_atomic_hist1D("/ALERT/TrackMatching/atof_electron_delta_phi_best", "Best #Delta#phi(e, ATOF hit);#Delta#phi [deg];Hits", 100, 150, 200);
            h_n_true_atof_hits = svc->book_atomic_hist1D("/ALERT/TrackMatching/n_true_atof_hits", "N true ATOF hits;N hits;Events", 20, 0, 20);

            // Debug histograms
            h_track_atof_dphi = svc->book_atomic_hist1D("/ALERT/Debug/track_atof_dphi", "Track-ATOF #Delta#phi (all true hits);|#Delta#phi| [deg];Hits", 100, 0, 180);
            h_best_track_atof_dphi = svc->book_atomic_hist1D("/ALERT/Debug/best_track_atof_dphi", "Best track-ATOF #Delta#phi (selected truth);|#Delta#phi| [deg];Events", 100, 0, 60);
            h_delta_component = svc->book_atomic_hist1D("/ALERT/Debug/delta_component", "AI - true component;#Delta component;Events", 41, -20, 20);
            h_delta_sector = svc->book_atomic_hist1D("/ALERT/Debug/delta_sector", "AI - true sector;#Delta sector;Events", 31, -15, 15);
            h_ai_vs_true_component = svc->book_atomic_hist2D("/ALERT/Debug/ai_vs_true_component", "AI vs true component;True component;AI component", 20, 0, 20, 20, 0, 20);
            h_ai_vs_true_layer = svc->book_atomic_hist2D("/ALERT/Debug/ai_vs_true_layer", "AI vs true layer;True layer;AI layer", 10, 0, 10, 10, 0, 10);
            h_ai_vs_true_sector = svc->book_atomic_hist2D("/ALERT/Debug/ai_vs_true_sector", "AI vs true sector;True sector;AI sector", 15, 0, 15, 15, 0, 15);
            h_track_phi_vs_atof_phi = svc->book_atomic_hist2D("/ALERT/Debug/track_phi_vs_atof_phi", "Track #phi vs best true ATOF #phi;True ATOF #phi [deg];Track #phi [deg]", 72, -180, 180, 72, -180, 180);

        });
    }

    struct ATOFWedgeH2Data {
        int sector, layer, component, order;
        double mean, sigma, a, b, c, d; 
    };

    std::vector<ATOFWedgeH2Data> m_atof_wedge_H2_data;

    // Efficiency counters (atomic for thread safety)
    std::atomic<int> m_n_tracks_matched_to_electron{0};   // denominator
    std::atomic<int> m_n_true_sample{0};                   // denominator: true track + true ATOF hit
    std::atomic<int> m_n_ai_has_match{0};                 // AI returned a valid ATOF hit id
    std::atomic<int> m_n_tracks_with_true_atof_match{0};  // numerator: AI match is the best true ATOF hit
    std::atomic<int> m_n_correct_sector{0};               // AI matched correct sector
    std::atomic<int> m_n_correct_layer{0};                // AI matched correct layer
    std::atomic<int> m_n_correct_component{0};            // AI matched correct component
    std::atomic<int> m_n_component_off_by_one{0};         // AI matched component within ±1
    std::atomic<int> m_n_correct_sector_layer{0};          // AI matched correct sector+layer
    std::atomic<int> m_n_correct_wedge{0};                // AI matched correct sector+layer+component
    std::atomic<int> m_n_wedge_off_by_one{0};             // AI matched sector+layer, component within ±1

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
