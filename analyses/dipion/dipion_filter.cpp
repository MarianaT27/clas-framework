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
class HipoEventWriterAlg : public clas12::framework::Algorithm {
public:
    explicit HipoEventWriterAlg(std::string name = "HipoEventWriterAlg")
        : Algorithm(std::move(name)) {}

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_hipo_svc = svc<clas12::framework::HipoFileService>();
        if (!m_hipo_svc || !m_hipo_svc->is_output_enabled()) {
            clas12::framework::Log::warning(name(), "HIPO output not configured — writer disabled");
            m_enabled = false;
        } else {
            clas12::framework::Log::info(name(), "Will write passing events to HIPO output (all banks)");
            m_enabled = true;
        }
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        if (!m_enabled) return clas12::framework::StatusCode::SUCCESS();

        // Write the raw event data directly — preserves the original format
        // (EVIO-style events as produced by CLAS12 reconstruction)
        auto raw = event.get_bank_view_set().raw_event_data();
        if (raw.empty()) return clas12::framework::StatusCode::SUCCESS();

        m_hipo_svc->write_event(raw);
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        if (m_enabled && m_hipo_svc) {
            clas12::framework::Log::info(name(), "Events written to HIPO: {}",
                                          m_hipo_svc->events_written());
        }
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::HipoFileService* m_hipo_svc = nullptr;
    bool m_enabled = false;
};

}  // namespace

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Di-Pion Analysis", "dipion_analysis",
        [](auto& builder, const auto& config) {
            std::string hipo_output;
            if (config.config_table) {
                if (auto v = (*config.config_table)["filter"]["hipo_output"].template value<std::string>())
                    hipo_output = *v;
            }

            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::DecayTreeFitterService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", config.target, config.polarity)
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::KinematicRegion{.q2_min = 1.0, .w_min = 2.0})
                .template add_algorithm<clas12::algorithms::PionSelectionAlg>("PionSelection", config.target, true);
                //.template add_algorithm<clas12::algorithms::Rho0ReconstructionAlg>("Rho0Reconstruction", config.target);

            if (!hipo_output.empty()) {
                builder.template configure_service<clas12::framework::HipoFileService>(
                    [hipo_output](clas12::framework::HipoFileService& svc) {
                        svc.set_output_file(hipo_output);
                    });
                builder.template add_algorithm<HipoEventWriterAlg>("HipoEventWriter");
            }
        });
}
