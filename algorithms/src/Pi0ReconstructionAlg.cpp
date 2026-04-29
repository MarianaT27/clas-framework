#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <Math/Vector4D.h>
#include <limits>
#include <string>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>


namespace algorithms {

using framework::Log;

Pi0ReconstructionAlg::Pi0ReconstructionAlg(std::string name, const Config& config)
    : Algorithm(std::move(name)), m_config(config), m_cutflow("Pi0 Reconstruction") {
    declare_config(m_config);
}

auto Pi0ReconstructionAlg::select_photons(const std::vector<Particle>& photons) const -> std::vector<Particle> {
    std::vector<Particle> selected;
    selected.reserve(photons.size());
    for (const auto& photon : photons) {
        if (photon.E() >= m_config.min_photon_energy) selected.push_back(photon);
    }
    return selected;
}

auto Pi0ReconstructionAlg::calculate_invariant_mass(const Particle& g1, const Particle& g2) -> double {
    return (g1.PxPyPzEVector() + g2.PxPyPzEVector()).M();
}

auto Pi0ReconstructionAlg::find_best_pi0(const std::vector<Particle>& photons) const -> std::optional<std::tuple<Particle, Particle, double>> {
    if (photons.size() < 2) return std::nullopt;

    std::optional<std::tuple<Particle, Particle, double>> best;
    double best_mass_diff = std::numeric_limits<double>::max();

    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            double mass = calculate_invariant_mass(photons[i], photons[j]);
            double mass_diff = std::abs(mass - PI0_MASS);
            if (mass_diff < m_config.pi0_mass_window && mass_diff < best_mass_diff) {
                best_mass_diff = mass_diff;
                best = std::make_tuple(photons[i], photons[j], mass);
            }
        }
    }

    return best;
}

auto Pi0ReconstructionAlg::find_all_unique_pi0_candidates(const std::vector<Particle>& photons) const -> std::optional<std::vector<std::tuple<Particle, Particle, double>>> {
    if (photons.size() < 2) return std::nullopt;

    std::vector<std::tuple<Particle, Particle, double>> candidates;
    std::vector<bool> used(photons.size(), false);

    while (true) {
        double best_mass_diff = std::numeric_limits<double>::max();
        size_t best_i = 0, best_j = 0;
        bool found_pair = false;

        for (size_t i = 0; i < photons.size(); ++i) {
            if (used[i]) continue;
            for (size_t j = i + 1; j < photons.size(); ++j) {
                if (used[j]) continue;
                double mass = calculate_invariant_mass(photons[i], photons[j]);
                double mass_diff = std::abs(mass - PI0_MASS);
                if (mass_diff < m_config.pi0_mass_window && mass_diff < best_mass_diff) {
                    best_mass_diff = mass_diff;
                    best_i = i;
                    best_j = j;
                    found_pair = true;
                }
            }
        }

        if (!found_pair) break;

        double mass = calculate_invariant_mass(photons[best_i], photons[best_j]);
        candidates.emplace_back(photons[best_i], photons[best_j], mass);
        used[best_i] = true;
        used[best_j] = true;
    }

    return candidates.empty() ? std::nullopt : std::make_optional(candidates);
}

auto Pi0ReconstructionAlg::on_initialize() -> StatusCode {
    Log::subsection(fmt::format("{} Configuration", name()));
    Log::info(name(), "Pi0 mass window: ±{} GeV", m_config.pi0_mass_window);

    m_cutflow.add_cut("All Events");
    m_cutflow.add_cut("Has Electron");
    m_cutflow.add_cut("Has >= 2 Photons");
    m_cutflow.add_cut("Pi0 Found");

    Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());
    return StatusCode::SUCCESS();
}

auto Pi0ReconstructionAlg::process(framework::Event& event) -> StatusCode {
    m_cutflow.pass("All Events");

    if (!event.passes_decision_typed(framework::Decisions::HasGoodDISElectron)) return StatusCode::FILTERED();
    m_cutflow.pass("Has Electron");

    auto photons_opt = event.retrieve_typed(framework::Keys::SelectedPhotons_Pi0);
    if (!photons_opt || photons_opt->size() < 2) return StatusCode::FILTERED();
    m_cutflow.pass("Has >= 2 Photons");

    const auto& selected_photons = *photons_opt;

    auto candidates_opt = find_all_unique_pi0_candidates(selected_photons);
    if (!candidates_opt.has_value()) return StatusCode::FILTERED();

    auto& tuple_candidates = candidates_opt.value();
    m_cutflow.pass("Pi0 Found");

    std::vector<physics::CompositeParticle> pi0_candidates;
    pi0_candidates.reserve(tuple_candidates.size());
    for (const auto& [g1, g2, mass] : tuple_candidates) {
        pi0_candidates.emplace_back(std::vector<Particle>{g1, g2}, 111);
    }

    event.store_typed(framework::Keys::Pi0Candidates, pi0_candidates);
    event.set_decision_typed(framework::Decisions::HasPi0, true);

    return StatusCode::SUCCESS();
}

auto Pi0ReconstructionAlg::on_finalize() -> StatusCode {
    Log::subsection(fmt::format("{} Summary", name()));
    Log::blank();
    m_cutflow.print();
    Log::success(name(), "Finalization complete");
    return StatusCode::SUCCESS();
}

}  // namespace algorithms
