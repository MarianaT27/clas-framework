/**
 * @file pi0_multiplicity.cpp
 * @brief Pi0 multiplicity ratio analysis with event-mixing background estimation
 *
 * Produces histograms binned in DIS kinematics (Q2, xB, z, pT2)
 * for computing the multiplicity ratio R_A = N_A^pi0 / N_D^pi0.
 * Also produces same-event and mixed-event invariant mass distributions
 * in 4D kinematic bins for background subtraction and signal extraction.
 *
 * Input is auto-detected: if the first file carries the slim PI0::event bank
 * (written by pi0_multiplicity_skim), a lightweight reader unpacks those banks
 * and the analysis skips the full electron/DIS/photon selection chain (and the
 * expensive Catboost photon AI). Otherwise the full chain runs on raw HIPO.
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/framework/services/HipoFileService.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/selection/PhotonSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <clas12/algorithms/analysis/Pi0AnalysisAlg.hpp>
#include <clas12/algorithms/analysis/Pi0SlimReaderAlg.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/services/PhotonCutsService.hpp>
#include <clas12/services/VzCorrectorService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Pi0 Multiplicity Ratio", "pi0_multiplicity",
        [](auto& builder, const auto& config) {
            double mass_window = 0.2;
            int    max_pool_events = 50;
            if (config.config_table) {
                if (auto v = (*config.config_table)["Pi0Reconstruction"]["mass_window"].template value<double>())
                    mass_window = *v;
                if (auto v = (*config.config_table)["Pi0Analysis"]["max_pool_events"].template value<int64_t>())
                    max_pool_events = static_cast<int>(*v);
            }

            // Auto-detect a slim skim: the PI0::event bank is only present in
            // files produced by pi0_multiplicity_skim.
            const bool slim = framework::HipoFileService::input_has_bank(
                config.input_files, std::string(bankdefs::PI0_event::bank_name));

            if (slim) {
                framework::Log::info("pi0_multiplicity",
                                     "Detected slim PI0 skim input — using slim reader "
                                     "(skipping electron/DIS/photon selection + AI)");
                builder
                    .template add_algorithm<algorithms::Pi0SlimReaderAlg>("Pi0SlimReader")
                    .template add_algorithm<algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", algorithms::Pi0ReconstructionConfig{.pi0_mass_window = mass_window})
                    .template add_algorithm<algorithms::Pi0AnalysisAlg>("Pi0Analysis", algorithms::Pi0AnalysisConfig{.max_pool_events = max_pool_events});
            } else {
                builder
                    .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                    .template add_service<framework::ElectronCutsService>()
                    .template add_service<framework::PhotonCutsService>()
                    .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                    .template add_algorithm<algorithms::ElectronSelectionAlg>()
                    .template add_algorithm<algorithms::DISSelectionAlg>("DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::standard_dis_region()})
                    .template add_algorithm<algorithms::PhotonSelectionAlg>("PhotonSelection")
                    .template add_algorithm<algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", algorithms::Pi0ReconstructionConfig{.pi0_mass_window = mass_window})
                    .template add_algorithm<algorithms::Pi0AnalysisAlg>("Pi0Analysis", algorithms::Pi0AnalysisConfig{.max_pool_events = max_pool_events});
            }
        });
}
