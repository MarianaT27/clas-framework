#pragma once

/**
 * @file ElasticEPAlg.hpp
 * @brief Algorithm for Elastic e-p Scattering analysis
 *
 */

#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/Particle.hpp>
#include <clas12/bankdefs/AHDC_track.hpp>
#include <clas12/bankdefs/ALERT_ai_projections.hpp>
#include <clas12/bankdefs/ATOF_hits.hpp>
#include <atomic>
#include <cmath>
#include <mutex>
#include <vector>
#include "Math/GenVector/PxPyPzE4D.h"
#include "Math/Vector3Dfwd.h"
#include <bank_view.h>

namespace ALERT_track_matching {

/// Configuration for ALERT_track_matchingAlg.
struct ALERT_track_matchingConfig {
    // Reserved for future tuning knobs that aren't user-tunable Properties.
};

/**
 * @brief Algorithm for ALERT track matching analysis
 */
class ALERT_track_matchingAlg : public framework::Algorithm {
public:
    using Config = ALERT_track_matchingConfig;
    using StatusCode = framework::StatusCode;

    explicit ALERT_track_matchingAlg(std::string name = "ALERT_track_matchingAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config) {
        required_services(m_hist_svc, m_ntuple_svc);
        declare_banks("REC::Particle", "AHDC::track", "ALERT::ai:projections", "ATOF::hits");
        declare_properties(m_example_cut, m_enable_histograms, m_phi_cut, m_delta_vz_cm_min, m_delta_vz_cm_max, m_enable_ntuple, m_ai_hit_id_offset);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    /**
     * @brief Configure service dependencies
     *
     * Called during initialization to declare required services.
     */
    /**
     * @brief Custom initialization
     *
     * Called after services are injected. Book histograms and setup resources here.
     * @return StatusCode SUCCESS to continue, FAILURE to abort
     */
    auto on_initialize() -> StatusCode override {
        if (m_enable_histograms.get()) book_histograms();
        if (m_enable_ntuple.get()) setup_ntuple();

        // Read the file analyses/ALERT_track_matching/atof_wedge_H2.data
        // Read the ATOF wedge H2 data file
            std::ifstream atof_file("analyses/ALERT_track_matching/atof_wedge_H2.data");
            if (!atof_file.is_open()) {
                framework::Log::error(name(), "Failed to open ATOF wedge H2 data file");
                return framework::StatusCode::FAILURE();
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

        return framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     *
     * Implement your per-event analysis logic here.
     * @param event The current event data
     * @return StatusCode SUCCESS to continue, FILTERED to skip, FAILURE to stop
     */
    auto process(framework::Event& event) -> framework::StatusCode override {

        // Check for elastic electron decision from upstream algorithm
        if (!event.passes_decision_typed(framework::Decisions::HasElasticElectron)) {
            return framework::StatusCode::FILTERED();
        }
        // Get elastic electron using typed retrieval
        auto electron_opt = event.retrieve_typed(framework::Keys::ElasticElectron);
        if (!electron_opt) {
            return framework::StatusCode::FILTERED();
        }
        const auto& elastic_electron = *electron_opt;
        double electron_phi = elastic_electron.phi();  // degrees

        // Bind generated wrappers for AHDC::track, ALERT::ai:projections
        // and ATOF::hits. bind() resolves every column handle once, so the
        // per-row getters below go through pre-resolved handles with zero
        // string lookups in the hot path.
        bankdefs::AHDC_track ahdc_tracks;
        bankdefs::ALERT_ai_projections alert_projections;
        bankdefs::ATOF_hits atof_hits;
        ahdc_tracks.bind(event.get_bank("AHDC::track"));
        alert_projections.bind(event.get_bank("ALERT::ai:projections"));
        atof_hits.bind(event.get_bank("ATOF::hits"));

        // Select good atof hits based on the m_atof_wedge_H2_data
        std::vector<int> good_atof_hit_indices;
        for (int i = 0; i < atof_hits.rows(); ++i) {
            const int sector = atof_hits.sector(i);
            const int layer = atof_hits.layer(i);
            const int component = atof_hits.component(i);
            const double time = static_cast<double>(atof_hits.time(i));
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
            const float hit_x = atof_hits.x(idx);
            const float hit_y = atof_hits.y(idx);
            const double atof_phi = std::atan2(hit_y, hit_x) * 180.0 / M_PI;

            double delta_phi = std::fmod(electron_phi - atof_phi, 360.0);
            if (delta_phi < 0.0) delta_phi += 360.0;

            h_atof_electron_delta_phi_all->fill(delta_phi);
            if (delta_phi >= 150.0 && delta_phi <= 200.0) {
                true_atof_hit_indices.push_back(idx);
                true_atof_hit_phis.push_back(atof_phi);
                h_atof_electron_delta_phi_true->fill(delta_phi);
            }
        }

        // Read AHDC tracks
        std::vector<AHDCTrackData> good_tracks;
        for (int i = 0; i < ahdc_tracks.rows(); ++i) {
            AHDCTrackData track;
            track.trackid       = ahdc_tracks.trackid(i);
            track.x             = ahdc_tracks.x(i);
            track.y             = ahdc_tracks.y(i);
            track.z             = ahdc_tracks.z(i);
            track.px            = ahdc_tracks.px(i);
            track.py            = ahdc_tracks.py(i);
            track.pz            = ahdc_tracks.pz(i);
            track.n_hits        = ahdc_tracks.n_hits(i);
            track.sum_adc       = ahdc_tracks.sum_adc(i);
            track.path          = ahdc_tracks.path(i);
            track.dEdx          = ahdc_tracks.dEdx(i);
            track.p_drift       = ahdc_tracks.p_drift(i);
            track.chi2          = ahdc_tracks.chi2(i);
            track.sum_residuals = ahdc_tracks.sum_residuals(i);
            good_tracks.push_back(track);
        }

        // Fill diagnostic histograms
        h_n_predictions->fill(good_tracks.size());
        h_n_good_atof_hits->fill(good_atof_hit_indices.size());
        h_n_true_atof_hits->fill(true_atof_hit_indices.size());
        if (good_tracks.empty() || good_atof_hit_indices.empty()) {
            return framework::StatusCode::FILTERED();
        }

        // Match tracks to the elastic electron using phi angle
        // For elastic ep, proton should be back-to-back with electron (150-200° apart)
        // Select tracks where delta_phi is between 150 and 200 degrees
        std::vector<const AHDCTrackData*> matched_tracks;

        double electron_vz = elastic_electron.vz() * 10.0;  // convert cm to mm

        for (const auto& track : good_tracks) {
            // Compute track phi from momentum
            double track_phi = std::atan2(track.py, track.px) * 180.0 / M_PI;

            double delta_phi = std::fmod(electron_phi - track_phi, 360.0);
            if (delta_phi < 0.0) delta_phi += 360.0;

            // Compute delta Vz between electron and track (mm then cm)
            double delta_vz = electron_vz - track.z;
            double delta_vz_cm = delta_vz / 10.0;

            // Fill histograms before cuts
            h_delta_phi->fill(delta_phi);
            h_delta_vz->fill(delta_vz);
            h_delta_vz_cm->fill(delta_vz_cm);
            // Fill delta_phi after delta Vz cm cut
            if (delta_vz_cm >= m_delta_vz_cm_min && delta_vz_cm <= m_delta_vz_cm_max) {
                h_delta_phi_after_vz_cm_cut->fill(delta_phi);
            }

            // Select tracks with delta_phi between 150 and 200 degrees and deltaVz within cm cut
            if (delta_phi >= 150.0 && delta_phi <= 200.0 && delta_vz_cm >= m_delta_vz_cm_min && delta_vz_cm <= m_delta_vz_cm_max) {
                matched_tracks.push_back(&track);
                h_best_delta_phi->fill(delta_phi);
            }
        }

        if (matched_tracks.empty() || matched_tracks.size() > 1) {
            return framework::StatusCode::FILTERED();
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

                h_track_atof_dphi->fill(dphi);
                if (dphi < best_phi_distance && dphi <= m_phi_cut) {
                    best_phi_distance = dphi;
                    best_true_atof_hit_index = true_atof_hit_indices[i];
                }
            }

            if (best_true_atof_hit_index >= 0) {
                const float hx = atof_hits.x(best_true_atof_hit_index);
                const float hy = atof_hits.y(best_true_atof_hit_index);
                const double best_atof_phi = std::atan2(hy, hx) * 180.0 / M_PI;
                double best_delta_phi = std::fmod(electron_phi - best_atof_phi, 360.0);
                if (best_delta_phi < 0.0) best_delta_phi += 360.0;
                h_atof_electron_delta_phi_best->fill(best_delta_phi);
                h_best_track_atof_dphi->fill(best_phi_distance);
                h_track_phi_vs_atof_phi->fill(best_atof_phi, track_phi);
            }
        }

        // Process the single matched track
        for (const auto* matched_track : matched_tracks) {
            ++m_n_tracks_matched_to_electron;

            // Find the ATOF hit associated with the matched track using ALERT::ai:projections
            int matched_atof_hit_id = -1;
            for (int i = 0; i < alert_projections.rows(); ++i) {
                const int proj_trackid = alert_projections.trackid(i);
                if (proj_trackid == matched_track->trackid) {
                    matched_atof_hit_id = alert_projections.matched_atof_hit_id(i) - m_ai_hit_id_offset;
                    break;
                }
            }

            h_matched_atof_hit_id->fill(matched_atof_hit_id);
            // Efficiency: only for the best true ATOF hit
            if (best_true_atof_hit_index >= 0) {
                ++m_n_true_sample;  // denominator: true track + true ATOF hit

                if (matched_atof_hit_id >= 0 && matched_atof_hit_id < atof_hits.rows()) {
                    ++m_n_ai_has_match;

                    const int ai_sector    = atof_hits.sector(matched_atof_hit_id);
                    const int ai_layer     = atof_hits.layer(matched_atof_hit_id);
                    const int ai_component = atof_hits.component(matched_atof_hit_id);

                    const int true_sector    = atof_hits.sector(best_true_atof_hit_index);
                    const int true_layer     = atof_hits.layer(best_true_atof_hit_index);
                    const int true_component = atof_hits.component(best_true_atof_hit_index);

                    h_delta_component->fill(ai_component - true_component);
                    h_delta_sector->fill(ai_sector - true_sector);
                    h_ai_vs_true_component->fill(true_component, ai_component);
                    h_ai_vs_true_layer->fill(true_layer, ai_layer);
                    h_ai_vs_true_sector->fill(true_sector, ai_sector);
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

        // Fill NTuple with per-event data for Python analysis
        if (m_enable_ntuple.get() && m_ntuple_svc) {
            {
                std::lock_guard<std::mutex> lock(m_ntuple_mutex);

                const auto* mt = matched_tracks.front();
                double trk_phi = std::atan2(mt->py, mt->px) * 180.0 / M_PI;
                double dphi_val = std::fmod(electron_phi - trk_phi, 360.0);
                if (dphi_val < 0.0) dphi_val += 360.0;

                // Electron
                m_nt_e_p     = elastic_electron.p();
                m_nt_e_theta = elastic_electron.theta();
                m_nt_e_phi   = elastic_electron.phi();
                m_nt_e_vz    = electron_vz;  // already in mm

                // Track
                m_nt_trackid             = mt->trackid;
                m_nt_track_x             = mt->x;
                m_nt_track_y             = mt->y;
                m_nt_track_z             = mt->z;
                m_nt_track_px            = mt->px;
                m_nt_track_py            = mt->py;
                m_nt_track_pz            = mt->pz;
                m_nt_track_p_drift       = mt->p_drift;
                m_nt_track_n_hits        = mt->n_hits;
                m_nt_track_sum_adc       = mt->sum_adc;
                m_nt_track_path          = mt->path;
                m_nt_track_dEdx          = mt->dEdx;
                m_nt_track_chi2          = mt->chi2;
                m_nt_track_sum_residuals = mt->sum_residuals;

                // Matching
                m_nt_delta_phi = dphi_val;
                m_nt_delta_vz  = electron_vz - mt->z;
                m_nt_track_phi = trk_phi;

                // Multiplicity
                m_nt_n_tracks         = static_cast<int>(good_tracks.size());
                m_nt_n_good_atof_hits = static_cast<int>(good_atof_hit_indices.size());
                m_nt_n_true_atof_hits = static_cast<int>(true_atof_hit_indices.size());
                m_nt_n_matched_tracks = static_cast<int>(matched_tracks.size());

                // True ATOF hit info
                m_nt_has_true_atof_hit = (best_true_atof_hit_index >= 0) ? 1 : 0;
                if (best_true_atof_hit_index >= 0) {
                    m_nt_true_sector    = atof_hits.sector(best_true_atof_hit_index);
                    m_nt_true_layer     = atof_hits.layer(best_true_atof_hit_index);
                    m_nt_true_component = atof_hits.component(best_true_atof_hit_index);
                    const float hx = atof_hits.x(best_true_atof_hit_index);
                    const float hy = atof_hits.y(best_true_atof_hit_index);
                    m_nt_true_atof_phi     = std::atan2(hy, hx) * 180.0 / M_PI;
                    m_nt_best_phi_distance = best_phi_distance;
                } else {
                    m_nt_true_sector       = -1;
                    m_nt_true_layer        = -1;
                    m_nt_true_component    = -1;
                    m_nt_true_atof_phi     = -999.0;
                    m_nt_best_phi_distance = -1.0;
                }

                // AI ATOF hit info
                int ai_hit_id = -1;
                for (int i = 0; i < alert_projections.rows(); ++i) {
                    if (alert_projections.trackid(i) == mt->trackid) {
                        ai_hit_id = alert_projections.matched_atof_hit_id(i) - m_ai_hit_id_offset;
                        break;
                    }
                }
                m_nt_matched_atof_hit_id = ai_hit_id;
                m_nt_has_ai_match = (ai_hit_id >= 0 && ai_hit_id < atof_hits.rows()) ? 1 : 0;

                if (m_nt_has_ai_match) {
                    m_nt_ai_sector    = atof_hits.sector(ai_hit_id);
                    m_nt_ai_layer     = atof_hits.layer(ai_hit_id);
                    m_nt_ai_component = atof_hits.component(ai_hit_id);
                } else {
                    m_nt_ai_sector    = -1;
                    m_nt_ai_layer     = -1;
                    m_nt_ai_component = -1;
                }

                // Efficiency flags
                if (m_nt_has_true_atof_hit && m_nt_has_ai_match) {
                    m_nt_is_sector_correct       = (m_nt_ai_sector == m_nt_true_sector) ? 1 : 0;
                    m_nt_is_layer_correct        = (m_nt_ai_layer == m_nt_true_layer) ? 1 : 0;
                    m_nt_is_component_correct    = (m_nt_ai_component == m_nt_true_component) ? 1 : 0;
                    m_nt_is_component_off_by_one = (std::abs(m_nt_ai_component - m_nt_true_component) <= 1) ? 1 : 0;
                    m_nt_is_wedge_correct        = (m_nt_ai_sector == m_nt_true_sector &&
                                                    m_nt_ai_layer == m_nt_true_layer &&
                                                    m_nt_ai_component == m_nt_true_component) ? 1 : 0;
                    m_nt_is_wedge_off_by_one     = (m_nt_ai_sector == m_nt_true_sector &&
                                                    m_nt_ai_layer == m_nt_true_layer &&
                                                    std::abs(m_nt_ai_component - m_nt_true_component) <= 1) ? 1 : 0;
                    m_nt_is_exact_match          = (ai_hit_id == best_true_atof_hit_index) ? 1 : 0;
                } else {
                    m_nt_is_sector_correct       = 0;
                    m_nt_is_layer_correct        = 0;
                    m_nt_is_component_correct    = 0;
                    m_nt_is_component_off_by_one = 0;
                    m_nt_is_wedge_correct        = 0;
                    m_nt_is_wedge_off_by_one     = 0;
                    m_nt_is_exact_match          = 0;
                }

                m_ntuple_svc->fill("alert_track_matching");
            }
        }

        return framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Custom finalization
     *
     * Called after all events. Print statistics and cleanup here.
     * @return StatusCode SUCCESS or FAILURE
     */
    auto on_finalize() -> framework::StatusCode override {
        // Print AI ATOF matching efficiency
        using Log = framework::Log;

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

        return framework::StatusCode::SUCCESS();
    }

private:
    // Configurable properties
    framework::Property<double> m_example_cut{
        "example_cut", 1.0, "Example cut threshold value"
    };
    framework::Property<bool> m_enable_histograms{
        "enable_histograms", true, "Enable histogram filling"
    };
    framework::Property<double> m_phi_cut{
        "phi_cut", 30.0, "Phi matching tolerance in degrees"
    };
    framework::Property<double> m_delta_vz_cm_min{
        "delta_vz_cm_min", 12.38, "Delta Vz minimum cut in cm"
    };
    framework::Property<double> m_delta_vz_cm_max{
        "delta_vz_cm_max", 37.26, "Delta Vz maximum cut in cm"
    };
    framework::Property<int> m_ai_hit_id_offset{
        "ai_hit_id_offset", 1, "Offset to subtract from AI matched_atof_hit_id (0 if 0-based, 1 if 1-based)"
    };

    framework::Property<bool> m_enable_ntuple{
        "enable_ntuple", true, "Enable N-tuple output"
    };

    Config m_config;

    // Services
    framework::ServiceRef<framework::HistogramService> m_hist_svc;
    framework::ServiceRef<framework::NTupleService> m_ntuple_svc;

    // Mutex for thread-safe NTuple filling
    mutable std::mutex m_ntuple_mutex;

    // Lock-free atomic histograms (managed by HistogramService)
    framework::ThreadLocalHistogram1D* h_p = nullptr;
    framework::ThreadLocalHistogram1D* h_phi = nullptr;
    framework::ThreadLocalHistogram1D* h_theta = nullptr;
    framework::ThreadLocalHistogram1D* h_vz = nullptr;
    framework::ThreadLocalHistogram1D* h_vz_Cu = nullptr;
    framework::ThreadLocalHistogram1D* h_vz_Sn = nullptr;
    framework::ThreadLocalHistogram1D* h_delta_vz = nullptr;
    framework::ThreadLocalHistogram1D* h_delta_vz_cm = nullptr;
    framework::ThreadLocalHistogram1D* h_delta_phi = nullptr;
    framework::ThreadLocalHistogram1D* h_delta_phi_after_vz_cm_cut = nullptr;
    framework::ThreadLocalHistogram1D* h_n_predictions = nullptr;
    framework::ThreadLocalHistogram1D* h_best_delta_phi = nullptr;
    framework::ThreadLocalHistogram1D* h_n_good_atof_hits = nullptr;
    framework::ThreadLocalHistogram1D* h_matched_atof_hit_id = nullptr;

    // ATOF-electron delta_phi histograms
    framework::ThreadLocalHistogram1D* h_atof_electron_delta_phi_all = nullptr;   // all good ATOF hits
    framework::ThreadLocalHistogram1D* h_atof_electron_delta_phi_true = nullptr;  // true ATOF hits (passing delta_phi cut)
    framework::ThreadLocalHistogram1D* h_atof_electron_delta_phi_best = nullptr;  // best true ATOF hit
    framework::ThreadLocalHistogram1D* h_n_true_atof_hits = nullptr;

    // Debug histograms
    framework::ThreadLocalHistogram1D* h_track_atof_dphi = nullptr;          // track-ATOF phi distance for all true hits
    framework::ThreadLocalHistogram1D* h_best_track_atof_dphi = nullptr;     // best track-ATOF phi distance (selected truth)
    framework::ThreadLocalHistogram1D* h_delta_component = nullptr;          // AI component - true component
    framework::ThreadLocalHistogram1D* h_delta_sector = nullptr;             // AI sector - true sector
    framework::ThreadLocalHistogram2D* h_ai_vs_true_component = nullptr;     // AI component vs true component
    framework::ThreadLocalHistogram2D* h_ai_vs_true_layer = nullptr;         // AI layer vs true layer
    framework::ThreadLocalHistogram2D* h_ai_vs_true_sector = nullptr;        // AI sector vs true sector
    framework::ThreadLocalHistogram2D* h_track_phi_vs_atof_phi = nullptr;    // track phi vs best true ATOF hit phi

    // NTuple branch variables
    double m_nt_e_p = 0.0, m_nt_e_theta = 0.0, m_nt_e_phi = 0.0, m_nt_e_vz = 0.0;

    int    m_nt_trackid = 0;
    double m_nt_track_x = 0.0, m_nt_track_y = 0.0, m_nt_track_z = 0.0;
    double m_nt_track_px = 0.0, m_nt_track_py = 0.0, m_nt_track_pz = 0.0;
    double m_nt_track_p_drift = 0.0;
    int    m_nt_track_n_hits = 0, m_nt_track_sum_adc = 0;
    double m_nt_track_path = 0.0, m_nt_track_dEdx = 0.0;
    double m_nt_track_chi2 = 0.0, m_nt_track_sum_residuals = 0.0;

    double m_nt_delta_phi = 0.0, m_nt_delta_vz = 0.0, m_nt_track_phi = 0.0;

    int    m_nt_true_sector = -1, m_nt_true_layer = -1, m_nt_true_component = -1;
    double m_nt_true_atof_phi = 0.0, m_nt_best_phi_distance = -1.0;

    int    m_nt_ai_sector = -1, m_nt_ai_layer = -1, m_nt_ai_component = -1;
    int    m_nt_matched_atof_hit_id = -1;

    int    m_nt_is_sector_correct = 0, m_nt_is_layer_correct = 0;
    int    m_nt_is_component_correct = 0, m_nt_is_component_off_by_one = 0;
    int    m_nt_is_wedge_correct = 0, m_nt_is_wedge_off_by_one = 0;
    int    m_nt_is_exact_match = 0, m_nt_has_ai_match = 0, m_nt_has_true_atof_hit = 0;

    int    m_nt_n_tracks = 0, m_nt_n_good_atof_hits = 0;
    int    m_nt_n_true_atof_hits = 0, m_nt_n_matched_tracks = 0;

    // NOTE: This algorithm currently registers ~40 branches via the legacy
    // svc->register_branch path. Migration to NTupleWriter<HadronTreeData>
    // (with static ntuple_fields()) is mechanical and is deferred to keep
    // this uniformization pass focused on constructor/service shape.
    auto setup_ntuple() -> void {
        if (!m_ntuple_svc) return;
        auto* svc = m_ntuple_svc.get();
        const std::string tree = "alert_track_matching";

        // Electron
        svc->register_branch(tree, "e_p",     &m_nt_e_p);
        svc->register_branch(tree, "e_theta", &m_nt_e_theta);
        svc->register_branch(tree, "e_phi",   &m_nt_e_phi);
        svc->register_branch(tree, "e_vz",    &m_nt_e_vz);

        // Track
        svc->register_branch(tree, "trackid",             &m_nt_trackid);
        svc->register_branch(tree, "track_x",             &m_nt_track_x);
        svc->register_branch(tree, "track_y",             &m_nt_track_y);
        svc->register_branch(tree, "track_z",             &m_nt_track_z);
        svc->register_branch(tree, "track_px",            &m_nt_track_px);
        svc->register_branch(tree, "track_py",            &m_nt_track_py);
        svc->register_branch(tree, "track_pz",            &m_nt_track_pz);
        svc->register_branch(tree, "track_p_drift",       &m_nt_track_p_drift);
        svc->register_branch(tree, "track_n_hits",        &m_nt_track_n_hits);
        svc->register_branch(tree, "track_sum_adc",       &m_nt_track_sum_adc);
        svc->register_branch(tree, "track_path",          &m_nt_track_path);
        svc->register_branch(tree, "track_dEdx",          &m_nt_track_dEdx);
        svc->register_branch(tree, "track_chi2",          &m_nt_track_chi2);
        svc->register_branch(tree, "track_sum_residuals", &m_nt_track_sum_residuals);

        // Matching
        svc->register_branch(tree, "delta_phi",  &m_nt_delta_phi);
        svc->register_branch(tree, "delta_vz",   &m_nt_delta_vz);
        svc->register_branch(tree, "track_phi",  &m_nt_track_phi);

        // True ATOF hit
        svc->register_branch(tree, "true_sector",       &m_nt_true_sector);
        svc->register_branch(tree, "true_layer",        &m_nt_true_layer);
        svc->register_branch(tree, "true_component",    &m_nt_true_component);
        svc->register_branch(tree, "true_atof_phi",     &m_nt_true_atof_phi);
        svc->register_branch(tree, "best_phi_distance", &m_nt_best_phi_distance);

        // AI ATOF hit
        svc->register_branch(tree, "ai_sector",           &m_nt_ai_sector);
        svc->register_branch(tree, "ai_layer",            &m_nt_ai_layer);
        svc->register_branch(tree, "ai_component",        &m_nt_ai_component);
        svc->register_branch(tree, "matched_atof_hit_id", &m_nt_matched_atof_hit_id);

        // Efficiency flags
        svc->register_branch(tree, "is_sector_correct",       &m_nt_is_sector_correct);
        svc->register_branch(tree, "is_layer_correct",        &m_nt_is_layer_correct);
        svc->register_branch(tree, "is_component_correct",    &m_nt_is_component_correct);
        svc->register_branch(tree, "is_component_off_by_one", &m_nt_is_component_off_by_one);
        svc->register_branch(tree, "is_wedge_correct",        &m_nt_is_wedge_correct);
        svc->register_branch(tree, "is_wedge_off_by_one",     &m_nt_is_wedge_off_by_one);
        svc->register_branch(tree, "is_exact_match",          &m_nt_is_exact_match);
        svc->register_branch(tree, "has_ai_match",            &m_nt_has_ai_match);
        svc->register_branch(tree, "has_true_atof_hit",       &m_nt_has_true_atof_hit);

        // Multiplicity
        svc->register_branch(tree, "n_tracks",          &m_nt_n_tracks);
        svc->register_branch(tree, "n_good_atof_hits",  &m_nt_n_good_atof_hits);
        svc->register_branch(tree, "n_true_atof_hits",  &m_nt_n_true_atof_hits);
        svc->register_branch(tree, "n_matched_tracks",  &m_nt_n_matched_tracks);
    }

    auto book_histograms() -> void {
        // Use lock-free atomic histograms for parallel accumulation
        auto* svc = m_hist_svc.get();
        h_p = svc->book_hist1D("/MC/Electron/p", "p;p;Events", 100, 0, 10);
        h_phi = svc->book_hist1D("/MC/Electron/phi", "#phi;#phi;Events", 100, -180, 180);
        h_theta = svc->book_hist1D("/MC/Electron/theta", "#theta;#theta;Events", 100, 0, 90);
        h_vz = svc->book_hist1D("/MC/Electron/vz", "vz;vz;Events", 100, -30, 30);
        h_vz_Cu = svc->book_hist1D("/MC/Electron/vz_Cu", "vz_Cu;vz_Cu;Events", 100, -30, 30);
        h_vz_Sn = svc->book_hist1D("/MC/Electron/vz_Sn", "vz_Sn;vz_Sn;Events", 100, -30, 30);
        h_delta_vz = svc->book_hist1D("/ALERT/TrackMatching/delta_vz", "#DeltaVz (electron - track);#DeltaVz [mm];Events", 200, -100, 100);
        h_delta_vz_cm = svc->book_hist1D("/ALERT/TrackMatching/delta_vz_cm", "#DeltaVz (electron - track);#DeltaVz [cm];Events", 200, -10, 50);
        h_delta_phi = svc->book_hist1D("/ALERT/TrackMatching/delta_phi", "#Delta#phi;#Delta#phi [deg];Events", 360, 0, 360);
        h_delta_phi_after_vz_cm_cut = svc->book_hist1D("/ALERT/TrackMatching/delta_phi_after_vz_cm_cut", "#Delta#phi after #DeltaVz cut;#Delta#phi [deg];Events", 360, 0, 360);
        h_n_predictions = svc->book_hist1D("/ALERT/TrackMatching/n_predictions", "N tracks;N tracks;Events", 20, 0, 20);
        h_best_delta_phi = svc->book_hist1D("/ALERT/TrackMatching/best_delta_phi", "Best #Delta#phi (track-electron);#Delta#phi [deg];Events", 100, 150, 200);
        h_n_good_atof_hits = svc->book_hist1D("/ALERT/TrackMatching/n_good_atof_hits", "N good ATOF hits;N hits;Events", 20, 0, 20);
        h_matched_atof_hit_id = svc->book_hist1D("/ALERT/TrackMatching/matched_atof_hit_id", "Matched ATOF hit ID;Hit ID;Events", 50, -1, 49);

        // ATOF-electron delta_phi histograms
        h_atof_electron_delta_phi_all = svc->book_hist1D("/ALERT/TrackMatching/atof_electron_delta_phi_all", "#Delta#phi(e, ATOF hit) all good;#Delta#phi [deg];Hits", 360, 0, 360);
        h_atof_electron_delta_phi_true = svc->book_hist1D("/ALERT/TrackMatching/atof_electron_delta_phi_true", "#Delta#phi(e, ATOF hit) true;#Delta#phi [deg];Hits", 100, 150, 200);
        h_atof_electron_delta_phi_best = svc->book_hist1D("/ALERT/TrackMatching/atof_electron_delta_phi_best", "Best #Delta#phi(e, ATOF hit);#Delta#phi [deg];Hits", 100, 150, 200);
        h_n_true_atof_hits = svc->book_hist1D("/ALERT/TrackMatching/n_true_atof_hits", "N true ATOF hits;N hits;Events", 20, 0, 20);

        // Debug histograms
        h_track_atof_dphi = svc->book_hist1D("/ALERT/Debug/track_atof_dphi", "Track-ATOF #Delta#phi (all true hits);|#Delta#phi| [deg];Hits", 100, 0, 180);
        h_best_track_atof_dphi = svc->book_hist1D("/ALERT/Debug/best_track_atof_dphi", "Best track-ATOF #Delta#phi (selected truth);|#Delta#phi| [deg];Events", 100, 0, 60);
        h_delta_component = svc->book_hist1D("/ALERT/Debug/delta_component", "AI - true component;#Delta component;Events", 41, -20, 20);
        h_delta_sector = svc->book_hist1D("/ALERT/Debug/delta_sector", "AI - true sector;#Delta sector;Events", 31, -15, 15);
        h_ai_vs_true_component = svc->book_hist2D("/ALERT/Debug/ai_vs_true_component", "AI vs true component;True component;AI component", 20, 0, 20, 20, 0, 20);
        h_ai_vs_true_layer = svc->book_hist2D("/ALERT/Debug/ai_vs_true_layer", "AI vs true layer;True layer;AI layer", 10, 0, 10, 10, 0, 10);
        h_ai_vs_true_sector = svc->book_hist2D("/ALERT/Debug/ai_vs_true_sector", "AI vs true sector;True sector;AI sector", 15, 0, 15, 15, 0, 15);
        h_track_phi_vs_atof_phi = svc->book_hist2D("/ALERT/Debug/track_phi_vs_atof_phi", "Track #phi vs best true ATOF #phi;True ATOF #phi [deg];Track #phi [deg]", 72, -180, 180, 72, -180, 180);
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
