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
        // Get elastic electron using typed retrieval
        std::optional<std::vector<clas12::Particle>> electrons_opt = event.retrieve("ElasticElectrons");
        if (!electrons_opt || electrons_opt->empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        const auto& elastic_electron = electrons_opt->front();


        // Read the AHDC::ai:prediction, ALERT::ai:projections and ATOF::hits banks
        hipo::bank& ahdc_prediction_bank = event.hipo_event().get("AHDC::ai:prediction");
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

        // Check if a good track is found in AHDC predictions
        std::vector<AHDCPredictionData> good_tracks;
        for (int i = 0; i < ahdc_prediction_bank.rows(); ++i) {
            int pred = ahdc_prediction_bank.get<int>("pred", i);
            if (pred > 0.2) {
                double x1 = ahdc_prediction_bank.get<double>("x1", i);
                double x2 = ahdc_prediction_bank.get<double>("x2", i);
                double x3 = ahdc_prediction_bank.get<double>("x3", i);
                double x4 = ahdc_prediction_bank.get<double>("x4", i);
                double x5 = ahdc_prediction_bank.get<double>("x5", i);
                double y1 = ahdc_prediction_bank.get<double>("y1", i);
                double y2 = ahdc_prediction_bank.get<double>("y2", i);
                double y3 = ahdc_prediction_bank.get<double>("y3", i);
                double y4 = ahdc_prediction_bank.get<double>("y4", i);
                double y5 = ahdc_prediction_bank.get<double>("y5", i);
                AHDCPredictionData track{ x1, x2, x3, x4, x5, y1, y2, y3, y4, y5, pred };
                good_tracks.push_back(track);
            }
        }

        if (good_tracks.empty() || good_atof_hit_indices.empty()) {
            clas12::framework::Log::info(name(), "No good tracks or ATOF hits found in this event");
            return clas12::framework::StatusCode::FILTERED();
        }

        // Check using the tracks match the one that is associated with the elastic electron (use phi and theta, of the first hits)








        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Custom finalization
     *
     * Called after all events. Print statistics and cleanup here.
     * @return StatusCode SUCCESS or FAILURE
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        // Print summary statistics
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

    // Service handles
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;

    // Lock-free atomic histograms (managed by HistogramService)
    clas12::framework::AtomicHistogram1D* h_p = nullptr;
    clas12::framework::AtomicHistogram1D* h_phi = nullptr;
    clas12::framework::AtomicHistogram1D* h_theta = nullptr;
    clas12::framework::AtomicHistogram1D* h_vz = nullptr;
    clas12::framework::AtomicHistogram1D* h_vz_Cu = nullptr;
    clas12::framework::AtomicHistogram1D* h_vz_Sn = nullptr;

    auto book_histograms() -> void {
        clas12::framework::safe(m_hist_svc).with([this](auto* svc) {
            // Use lock-free atomic histograms for parallel accumulation
            h_p = svc->book_atomic_hist1D("/MC/Electron/p", "p;p;Events", 100, 0, 10);
            h_phi = svc->book_atomic_hist1D("/MC/Electron/phi", "#phi;#phi;Events", 100, -180, 180);
            h_theta = svc->book_atomic_hist1D("/MC/Electron/theta", "#theta;#theta;Events", 100, 0, 90);
            h_vz = svc->book_atomic_hist1D("/MC/Electron/vz", "vz;vz;Events", 100, -30, 30);
            h_vz_Cu = svc->book_atomic_hist1D("/MC/Electron/vz_Cu", "vz_Cu;vz_Cu;Events", 100, -30, 30);
            h_vz_Sn = svc->book_atomic_hist1D("/MC/Electron/vz_Sn", "vz_Sn;vz_Sn;Events", 100, -30, 30);
        });
    }

    struct ATOFWedgeH2Data {
        int sector, layer, component, order;
        double mean, sigma, a, b, c, d; 
    };

    std::vector<ATOFWedgeH2Data> m_atof_wedge_H2_data;

    struct AHDCPredictionData {
        double x1, x2, x3, x4, x5, y1, y2, y3, y4, y5;
        int pred;
    };
};

}  // namespace myanalysis
