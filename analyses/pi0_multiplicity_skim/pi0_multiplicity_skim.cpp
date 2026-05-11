/**
 * @file pi0_multiplicity_skim.cpp
 * @brief Skim for pi0 multiplicity analysis: electron + DIS + at least one good photon
 *
 * Pre-filters events requiring:
 * 1. Good electron (chi2pid, momentum, vz, sampling fraction, fiducial)
 * 2. DIS kinematics (Q2 >= 1, W >= 2, y <= 0.85)
 * 3. At least one good photon (AI cuts + beta + PCAL fiducial)
 *
 * Passing events are written to a HIPO output file for downstream analysis.
 */

#include <string>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/selection/PhotonSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include <clas12/framework/services/VzCorrectorService.hpp>
#include <clas12/framework/services/HipoFileService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace {

/**
 * @brief Algorithm that writes all passing events to the HIPO output via HipoFileService
 *
 * Placed last in the algorithm chain. Events that reach this algorithm have
 * passed all upstream cuts (electron + DIS + photon selection).
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

        auto raw = event.get_bank_view_set().raw_event_data();
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
        argc, argv, "Pi0 Multiplicity Skim", "pi0_multiplicity_skim",
        [](auto& builder, const auto& config) {
            // Optional HIPO output file path lives in the TOML at
            // [skim].hipo_output (top-level so it isn't tied to a
            // specific algorithm). Empty / missing = no output.
            std::string hipo_output;
            if (config.config_table) {
                if (auto v = (*config.config_table)["skim"]["hipo_output"].template value<std::string>())
                    hipo_output = *v;
            }

            builder
                .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                .template add_service<framework::ElectronCutsService>()
                .template add_service<framework::PhotonCutsService>()
                .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", algorithms::ElectronSelectionConfig{.target = config.target, .polarity = config.polarity})
                .template add_algorithm<algorithms::DISSelectionAlg>("DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::standard_dis_region()})
                .template add_algorithm<algorithms::PhotonSelectionAlg>("PhotonSelection");

            if (!hipo_output.empty()) {
                builder.template configure_service<framework::HipoFileService>(
                    [hipo_output](framework::HipoFileService& svc) {
                        svc.set_output_file(hipo_output);
                    });
                builder.template add_algorithm<HipoEventWriterAlg>("HipoEventWriter");
            }
        });
}
