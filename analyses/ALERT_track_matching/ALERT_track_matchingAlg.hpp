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
#include <clas12/framework/core/NTupleStruct.hpp>
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

    double example_cut = 1.0;
    bool enable_histograms = true;
    double phi_cut = 30.0;
    double delta_vz_cm_min = 12.38;
    double delta_vz_cm_max = 37.26;
    int ai_hit_id_offset = 1;
    bool enable_ntuple = true;

    static constexpr auto config_fields() {
        using T = ALERT_track_matchingConfig;
        return std::make_tuple(
            framework::field("example_cut", &T::example_cut),
            framework::field("enable_histograms", &T::enable_histograms),
            framework::field("phi_cut", &T::phi_cut),
            framework::field("delta_vz_cm_min", &T::delta_vz_cm_min),
            framework::field("delta_vz_cm_max", &T::delta_vz_cm_max),
            framework::field("ai_hit_id_offset", &T::ai_hit_id_offset),
            framework::field("enable_ntuple", &T::enable_ntuple));
    }
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
        declare_config(m_config);
        declare_histograms(h_p, h_phi, h_theta, h_vz, h_vz_Cu, h_vz_Sn, h_delta_vz, h_delta_vz_cm, h_delta_phi, h_delta_phi_after_vz_cm_cut, h_n_predictions, h_best_delta_phi, h_n_good_atof_hits, h_matched_atof_hit_id, h_n_true_atof_hits, h_atof_electron_delta_phi_all, h_atof_electron_delta_phi_true, h_atof_electron_delta_phi_best, h_track_atof_dphi, h_best_track_atof_dphi, h_delta_component, h_delta_sector, h_ai_vs_true_component, h_ai_vs_true_layer, h_ai_vs_true_sector, h_track_phi_vs_atof_phi);
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
        // Histograms self-register at initialize time via declare_histograms().
        if (m_config.enable_ntuple) m_writer.init(m_ntuple_svc.get(), "alert_track_matching");

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

            h_atof_electron_delta_phi_all.fill(delta_phi);
            if (delta_phi >= 150.0 && delta_phi <= 200.0) {
                true_atof_hit_indices.push_back(idx);
                true_atof_hit_phis.push_back(atof_phi);
                h_atof_electron_delta_phi_true.fill(delta_phi);
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
        h_n_predictions.fill(good_tracks.size());
        h_n_good_atof_hits.fill(good_atof_hit_indices.size());
        h_n_true_atof_hits.fill(true_atof_hit_indices.size());
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
            h_delta_phi.fill(delta_phi);
            h_delta_vz.fill(delta_vz);
            h_delta_vz_cm.fill(delta_vz_cm);
            // Fill delta_phi after delta Vz cm cut
            if (delta_vz_cm >= m_config.delta_vz_cm_min && delta_vz_cm <= m_config.delta_vz_cm_max) {
                h_delta_phi_after_vz_cm_cut.fill(delta_phi);
            }

            // Select tracks with delta_phi between 150 and 200 degrees and deltaVz within cm cut
            if (delta_phi >= 150.0 && delta_phi <= 200.0 && delta_vz_cm >= m_config.delta_vz_cm_min && delta_vz_cm <= m_config.delta_vz_cm_max) {
                matched_tracks.push_back(&track);
                h_best_delta_phi.fill(delta_phi);
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

                h_track_atof_dphi.fill(dphi);
                if (dphi < best_phi_distance && dphi <= m_config.phi_cut) {
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
                h_atof_electron_delta_phi_best.fill(best_delta_phi);
                h_best_track_atof_dphi.fill(best_phi_distance);
                h_track_phi_vs_atof_phi.fill(best_atof_phi, track_phi);
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
                    matched_atof_hit_id = alert_projections.matched_atof_hit_id(i) - m_config.ai_hit_id_offset;
                    break;
                }
            }

            h_matched_atof_hit_id.fill(matched_atof_hit_id);
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

                    h_delta_component.fill(ai_component - true_component);
                    h_delta_sector.fill(ai_sector - true_sector);
                    h_ai_vs_true_component.fill(true_component, ai_component);
                    h_ai_vs_true_layer.fill(true_layer, ai_layer);
                    h_ai_vs_true_sector.fill(true_sector, ai_sector);
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
        if (m_writer.is_initialized()) {
            std::lock_guard<std::mutex> lock(m_ntuple_mutex);

            auto& nt = m_writer.data();
            const auto* mt = matched_tracks.front();
            double trk_phi = std::atan2(mt->py, mt->px) * 180.0 / M_PI;
            double dphi_val = std::fmod(electron_phi - trk_phi, 360.0);
            if (dphi_val < 0.0) dphi_val += 360.0;

            nt.e_p     = elastic_electron.p();
            nt.e_theta = elastic_electron.theta();
            nt.e_phi   = elastic_electron.phi();
            nt.e_vz    = electron_vz;

            nt.trackid             = mt->trackid;
            nt.track_x             = mt->x;
            nt.track_y             = mt->y;
            nt.track_z             = mt->z;
            nt.track_px            = mt->px;
            nt.track_py            = mt->py;
            nt.track_pz            = mt->pz;
            nt.track_p_drift       = mt->p_drift;
            nt.track_n_hits        = mt->n_hits;
            nt.track_sum_adc       = mt->sum_adc;
            nt.track_path          = mt->path;
            nt.track_dEdx          = mt->dEdx;
            nt.track_chi2          = mt->chi2;
            nt.track_sum_residuals = mt->sum_residuals;

            nt.delta_phi = dphi_val;
            nt.delta_vz  = electron_vz - mt->z;
            nt.track_phi = trk_phi;

            nt.n_tracks         = static_cast<int>(good_tracks.size());
            nt.n_good_atof_hits = static_cast<int>(good_atof_hit_indices.size());
            nt.n_true_atof_hits = static_cast<int>(true_atof_hit_indices.size());
            nt.n_matched_tracks = static_cast<int>(matched_tracks.size());

            nt.has_true_atof_hit = (best_true_atof_hit_index >= 0) ? 1 : 0;
            if (best_true_atof_hit_index >= 0) {
                nt.true_sector    = atof_hits.sector(best_true_atof_hit_index);
                nt.true_layer     = atof_hits.layer(best_true_atof_hit_index);
                nt.true_component = atof_hits.component(best_true_atof_hit_index);
                const float hx = atof_hits.x(best_true_atof_hit_index);
                const float hy = atof_hits.y(best_true_atof_hit_index);
                nt.true_atof_phi     = std::atan2(hy, hx) * 180.0 / M_PI;
                nt.best_phi_distance = best_phi_distance;
            } else {
                nt.true_sector       = -1;
                nt.true_layer        = -1;
                nt.true_component    = -1;
                nt.true_atof_phi     = -999.0;
                nt.best_phi_distance = -1.0;
            }

            int ai_hit_id = -1;
            for (int i = 0; i < alert_projections.rows(); ++i) {
                if (alert_projections.trackid(i) == mt->trackid) {
                    ai_hit_id = alert_projections.matched_atof_hit_id(i) - m_config.ai_hit_id_offset;
                    break;
                }
            }
            nt.matched_atof_hit_id = ai_hit_id;
            nt.has_ai_match = (ai_hit_id >= 0 && ai_hit_id < atof_hits.rows()) ? 1 : 0;

            if (nt.has_ai_match) {
                nt.ai_sector    = atof_hits.sector(ai_hit_id);
                nt.ai_layer     = atof_hits.layer(ai_hit_id);
                nt.ai_component = atof_hits.component(ai_hit_id);
            } else {
                nt.ai_sector    = -1;
                nt.ai_layer     = -1;
                nt.ai_component = -1;
            }

            if (nt.has_true_atof_hit && nt.has_ai_match) {
                nt.is_sector_correct       = (nt.ai_sector == nt.true_sector) ? 1 : 0;
                nt.is_layer_correct        = (nt.ai_layer == nt.true_layer) ? 1 : 0;
                nt.is_component_correct    = (nt.ai_component == nt.true_component) ? 1 : 0;
                nt.is_component_off_by_one = (std::abs(nt.ai_component - nt.true_component) <= 1) ? 1 : 0;
                nt.is_wedge_correct        = (nt.ai_sector == nt.true_sector &&
                                              nt.ai_layer == nt.true_layer &&
                                              nt.ai_component == nt.true_component) ? 1 : 0;
                nt.is_wedge_off_by_one     = (nt.ai_sector == nt.true_sector &&
                                              nt.ai_layer == nt.true_layer &&
                                              std::abs(nt.ai_component - nt.true_component) <= 1) ? 1 : 0;
                nt.is_exact_match          = (ai_hit_id == best_true_atof_hit_index) ? 1 : 0;
            } else {
                nt.is_sector_correct       = 0;
                nt.is_layer_correct        = 0;
                nt.is_component_correct    = 0;
                nt.is_component_off_by_one = 0;
                nt.is_wedge_correct        = 0;
                nt.is_wedge_off_by_one     = 0;
                nt.is_exact_match          = 0;
                }

            m_writer.fill();
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
Config m_config;

    // Services
    framework::ServiceRef<framework::HistogramService> m_hist_svc;
    framework::ServiceRef<framework::NTupleService> m_ntuple_svc;

    // Mutex for thread-safe NTuple filling
    mutable std::mutex m_ntuple_mutex;

    // Lock-free atomic histograms (managed by HistogramService)
    framework::Hist1<> h_p{"/MC/Electron/p", "p;p;Events", 0, 10, 100};
    framework::Hist1<> h_phi{"/MC/Electron/phi", "#phi;#phi;Events", -180, 180, 100};
    framework::Hist1<> h_theta{"/MC/Electron/theta", "#theta;#theta;Events", 0, 90, 100};
    framework::Hist1<> h_vz{"/MC/Electron/vz", "vz;vz;Events", -30, 30, 100};
    framework::Hist1<> h_vz_Cu{"/MC/Electron/vz_Cu", "vz_Cu;vz_Cu;Events", -30, 30, 100};
    framework::Hist1<> h_vz_Sn{"/MC/Electron/vz_Sn", "vz_Sn;vz_Sn;Events", -30, 30, 100};
    framework::Hist1<> h_delta_vz{"/ALERT/TrackMatching/delta_vz", "#DeltaVz (electron - track);#DeltaVz [mm];Events", -100, 100, 200};
    framework::Hist1<> h_delta_vz_cm{"/ALERT/TrackMatching/delta_vz_cm", "#DeltaVz (electron - track);#DeltaVz [cm];Events", -10, 50, 200};
    framework::Hist1<> h_delta_phi{"/ALERT/TrackMatching/delta_phi", "#Delta#phi;#Delta#phi [deg];Events", 0, 360, 360};
    framework::Hist1<> h_delta_phi_after_vz_cm_cut{"/ALERT/TrackMatching/delta_phi_after_vz_cm_cut", "#Delta#phi after #DeltaVz cut;#Delta#phi [deg];Events", 0, 360, 360};
    framework::Hist1<> h_n_predictions{"/ALERT/TrackMatching/n_predictions", "N tracks;N tracks;Events", 0, 20, 20};
    framework::Hist1<> h_best_delta_phi{"/ALERT/TrackMatching/best_delta_phi", "Best #Delta#phi (track-electron);#Delta#phi [deg];Events", 150, 200, 100};
    framework::Hist1<> h_n_good_atof_hits{"/ALERT/TrackMatching/n_good_atof_hits", "N good ATOF hits;N hits;Events", 0, 20, 20};
    framework::Hist1<> h_matched_atof_hit_id{"/ALERT/TrackMatching/matched_atof_hit_id", "Matched ATOF hit ID;Hit ID;Events", -1, 49, 50};
    // ATOF-electron delta_phi histograms
    framework::Hist1<> h_atof_electron_delta_phi_all{"/ALERT/TrackMatching/atof_electron_delta_phi_all", "#Delta#phi(e, ATOF hit) all good;#Delta#phi [deg];Hits", 0, 360, 360};
    framework::Hist1<> h_atof_electron_delta_phi_true{"/ALERT/TrackMatching/atof_electron_delta_phi_true", "#Delta#phi(e, ATOF hit) true;#Delta#phi [deg];Hits", 150, 200, 100};
    framework::Hist1<> h_atof_electron_delta_phi_best{"/ALERT/TrackMatching/atof_electron_delta_phi_best", "Best #Delta#phi(e, ATOF hit);#Delta#phi [deg];Hits", 150, 200, 100};
    framework::Hist1<> h_n_true_atof_hits{"/ALERT/TrackMatching/n_true_atof_hits", "N true ATOF hits;N hits;Events", 0, 20, 20};
    // Debug histograms
    framework::Hist1<> h_track_atof_dphi{"/ALERT/Debug/track_atof_dphi", "Track-ATOF #Delta#phi (all true hits);|#Delta#phi| [deg];Hits", 0, 180, 100};
    framework::Hist1<> h_best_track_atof_dphi{"/ALERT/Debug/best_track_atof_dphi", "Best track-ATOF #Delta#phi (selected truth);|#Delta#phi| [deg];Events", 0, 60, 100};
    framework::Hist1<> h_delta_component{"/ALERT/Debug/delta_component", "AI - true component;#Delta component;Events", -20, 20, 41};
    framework::Hist1<> h_delta_sector{"/ALERT/Debug/delta_sector", "AI - true sector;#Delta sector;Events", -15, 15, 31};
    framework::Hist2<> h_ai_vs_true_component{"/ALERT/Debug/ai_vs_true_component", "AI vs true component;True component;AI component", 0, 20, 20, 0, 20, 20};
    framework::Hist2<> h_ai_vs_true_layer{"/ALERT/Debug/ai_vs_true_layer", "AI vs true layer;True layer;AI layer", 0, 10, 10, 0, 10, 10};
    framework::Hist2<> h_ai_vs_true_sector{"/ALERT/Debug/ai_vs_true_sector", "AI vs true sector;True sector;AI sector", 0, 15, 15, 0, 15, 15};
    framework::Hist2<> h_track_phi_vs_atof_phi{"/ALERT/Debug/track_phi_vs_atof_phi", "Track #phi vs best true ATOF #phi;True ATOF #phi [deg];Track #phi [deg]", -180, 180, 72, -180, 180, 72};

    // NTuple row type — branch names match the legacy register_branch list
    // exactly, so downstream macros keep working.
    struct AlertNTupleData {
        double e_p = 0.0;
        double e_theta = 0.0;
        double e_phi = 0.0;
        double e_vz = 0.0;

        int    trackid = 0;
        double track_x = 0.0;
        double track_y = 0.0;
        double track_z = 0.0;
        double track_px = 0.0;
        double track_py = 0.0;
        double track_pz = 0.0;
        double track_p_drift = 0.0;
        int    track_n_hits = 0;
        int    track_sum_adc = 0;
        double track_path = 0.0;
        double track_dEdx = 0.0;
        double track_chi2 = 0.0;
        double track_sum_residuals = 0.0;

        double delta_phi = 0.0;
        double delta_vz = 0.0;
        double track_phi = 0.0;

        int    true_sector = -1;
        int    true_layer = -1;
        int    true_component = -1;
        double true_atof_phi = 0.0;
        double best_phi_distance = -1.0;

        int    ai_sector = -1;
        int    ai_layer = -1;
        int    ai_component = -1;
        int    matched_atof_hit_id = -1;

        int    is_sector_correct = 0;
        int    is_layer_correct = 0;
        int    is_component_correct = 0;
        int    is_component_off_by_one = 0;
        int    is_wedge_correct = 0;
        int    is_wedge_off_by_one = 0;
        int    is_exact_match = 0;
        int    has_ai_match = 0;
        int    has_true_atof_hit = 0;

        int    n_tracks = 0;
        int    n_good_atof_hits = 0;
        int    n_true_atof_hits = 0;
        int    n_matched_tracks = 0;

        static constexpr auto ntuple_fields() {
            using T = AlertNTupleData;
            return std::make_tuple(
                framework::field("e_p",     &T::e_p),
                framework::field("e_theta", &T::e_theta),
                framework::field("e_phi",   &T::e_phi),
                framework::field("e_vz",    &T::e_vz),

                framework::field("trackid",             &T::trackid),
                framework::field("track_x",             &T::track_x),
                framework::field("track_y",             &T::track_y),
                framework::field("track_z",             &T::track_z),
                framework::field("track_px",            &T::track_px),
                framework::field("track_py",            &T::track_py),
                framework::field("track_pz",            &T::track_pz),
                framework::field("track_p_drift",       &T::track_p_drift),
                framework::field("track_n_hits",        &T::track_n_hits),
                framework::field("track_sum_adc",       &T::track_sum_adc),
                framework::field("track_path",          &T::track_path),
                framework::field("track_dEdx",          &T::track_dEdx),
                framework::field("track_chi2",          &T::track_chi2),
                framework::field("track_sum_residuals", &T::track_sum_residuals),

                framework::field("delta_phi", &T::delta_phi),
                framework::field("delta_vz",  &T::delta_vz),
                framework::field("track_phi", &T::track_phi),

                framework::field("true_sector",       &T::true_sector),
                framework::field("true_layer",        &T::true_layer),
                framework::field("true_component",    &T::true_component),
                framework::field("true_atof_phi",     &T::true_atof_phi),
                framework::field("best_phi_distance", &T::best_phi_distance),

                framework::field("ai_sector",           &T::ai_sector),
                framework::field("ai_layer",            &T::ai_layer),
                framework::field("ai_component",        &T::ai_component),
                framework::field("matched_atof_hit_id", &T::matched_atof_hit_id),

                framework::field("is_sector_correct",       &T::is_sector_correct),
                framework::field("is_layer_correct",        &T::is_layer_correct),
                framework::field("is_component_correct",    &T::is_component_correct),
                framework::field("is_component_off_by_one", &T::is_component_off_by_one),
                framework::field("is_wedge_correct",        &T::is_wedge_correct),
                framework::field("is_wedge_off_by_one",     &T::is_wedge_off_by_one),
                framework::field("is_exact_match",          &T::is_exact_match),
                framework::field("has_ai_match",            &T::has_ai_match),
                framework::field("has_true_atof_hit",       &T::has_true_atof_hit),

                framework::field("n_tracks",         &T::n_tracks),
                framework::field("n_good_atof_hits", &T::n_good_atof_hits),
                framework::field("n_true_atof_hits", &T::n_true_atof_hits),
                framework::field("n_matched_tracks", &T::n_matched_tracks));
        }
    };

    framework::NTupleWriter<AlertNTupleData> m_writer;

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
