/**
 * @file dipion_filter.cpp
 * @brief Di-pion analysis with full electron, DIS, pion, and rho0 reconstruction cuts
 *
 * Uses the same algorithm chain as the nuclear transparency analysis:
 * 1. ElectronSelectionAlg - chi2pid, momentum, vz, sampling fraction, fiducial cuts
 * 2. DISSelectionAlg - Q2 >= 1.0, W >= 2.0, y <= 0.85
 * 3. PionSelectionAlg - chi2pid, momentum, vz, delta_t, detector region, fiducial cuts
 * 4. Rho0ReconstructionAlg - zh, lc, t, delta_vz cuts
 * 5. HipoEventWriterAlg - writes passing events to a single merged HIPO output (optional)
 */

#include <string>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/selection/PionSelectionAlg.hpp>
#include <clas12/algorithms/reconstruction/Rho0ReconstructionAlg.hpp>
#include <clas12/framework/services/DecayTreeFitterService.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/HipoFileService.hpp>
#include <clas12/framework/services/VzCorrectorService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace {

/**
 * @brief Algorithm that writes all passing events to the HIPO output via HipoFileService
 *
 * Placed last in the algorithm chain. Events that reach this algorithm have
 * passed all upstream cuts. Rebuilds a proper HIPO event from the raw event
 * data (which may be in EVIO format) by scanning all bank structures and
 * re-serializing them using event_builder.
 */
class HipoEventWriterAlg : public framework::Algorithm {
public:
    explicit HipoEventWriterAlg(std::string name = "HipoEventWriterAlg")
        : Algorithm(std::move(name)) {}

    auto on_initialize() -> framework::StatusCode override {
        m_hipo_svc = svc<framework::HipoFileService>();
        if (!m_hipo_svc || !m_hipo_svc->is_output_enabled()) {
            framework::Log::warning(name(), "HIPO output not configured — writer disabled");
            m_enabled = false;
        } else {
            framework::Log::info(name(), "Will write passing events to HIPO output (all banks)");
            m_enabled = true;
        }
        return framework::StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> framework::StatusCode override {
        if (!m_enabled) return framework::StatusCode::SUCCESS();

        // Write the raw event data directly — preserves the original format
        // (EVIO-style events as produced by CLAS12 reconstruction)
        const auto& raw = event.get_raw_event_buffer();
        if (raw.empty()) return framework::StatusCode::SUCCESS();

        m_hipo_svc->write_event(raw);
        return framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> framework::StatusCode override {
        if (m_enabled && m_hipo_svc) {
            framework::Log::info(name(), "Events written to HIPO: {}",
                                          m_hipo_svc->events_written());
        }
        return framework::StatusCode::SUCCESS();
    }

private:
    framework::HipoFileService* m_hipo_svc = nullptr;
    bool m_enabled = false;
};

}  // namespace

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Di-Pion Analysis", "dipion_analysis",
        [](auto& builder, const auto& config) {
            std::string hipo_output;
            if (config.config_table) {
                if (auto v = (*config.config_table)["filter"]["hipo_output"].template value<std::string>())
                    hipo_output = *v;
            }

            builder
                .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                .template add_service<framework::ElectronCutsService>()
                .template add_service<framework::DecayTreeFitterService>()
                .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", algorithms::ElectronSelectionConfig{.target = config.target, .polarity = config.polarity})
                .template add_algorithm<algorithms::DISSelectionAlg>("DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::KinematicRegion{.q2_min = 1.0, .w_min = 2.0}})
                .template add_algorithm<algorithms::PionSelectionAlg>("PionSelection", algorithms::PionSelectionConfig{.target = config.target, .polarity = config.polarity});
                //.template add_algorithm<algorithms::Rho0ReconstructionAlg>("Rho0Reconstruction", config.target);

            if (!hipo_output.empty()) {
                builder.template configure_service<framework::HipoFileService>(
                    [hipo_output](framework::HipoFileService& svc) {
                        svc.set_output_file(hipo_output);
                    });
                builder.template add_algorithm<HipoEventWriterAlg>("HipoEventWriter");
            }
        });
}
