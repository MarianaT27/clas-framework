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
#include <clas12/framework/services/FiducialService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include <clas12/framework/services/HipoFileService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace {

/**
 * @brief Algorithm that writes all passing events to the HIPO output via HipoFileService
 *
 * Placed last in the algorithm chain. Events that reach this algorithm have
 * passed all upstream cuts (electron + DIS + photon selection).
 */
class HipoEventWriterAlg : public clas12::framework::SimpleAlgorithm {
public:
    explicit HipoEventWriterAlg(std::string name = "HipoEventWriterAlg")
        : SimpleAlgorithm(std::move(name)) {}

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
        argc, argv, "Pi0 Multiplicity Skim", "pi0_multiplicity_skim",
        [](auto& builder, const auto& config) {
            const std::string hipo_output =
                config.custom["hipo-output"].template value<std::string>().value_or(std::string{});

            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::FiducialService>()
                .template add_service<clas12::framework::PhotonCutsService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", config.target, config.polarity)
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::standard_dis_region())
                .template add_algorithm<clas12::algorithms::PhotonSelectionAlg>("PhotonSelection");

            // If HIPO output requested, configure the service and add the writer algorithm
            if (!hipo_output.empty()) {
                builder.template configure_service<clas12::framework::HipoFileService>(
                    [hipo_output](clas12::framework::HipoFileService& svc) {
                        svc.set_output_file(hipo_output);
                    });
                builder.template add_algorithm<HipoEventWriterAlg>("HipoEventWriter");
            }
        },
        // Extra CLI options
        [](cxxopts::Options& opts) {
            opts.add_options()
                ("hipo-output", "Write passing events to a single merged HIPO output file",
                    cxxopts::value<std::string>()->default_value(""));
        },
        // Parse extra options
        [](const cxxopts::ParseResult& args, clas12::framework::AnalysisConfig& config) {
            config.custom.insert_or_assign("hipo-output", args["hipo-output"].as<std::string>());
        });
}
