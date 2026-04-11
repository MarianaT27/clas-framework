#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <Math/Vector4D.h>
#include <limits>
#include <string>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/Pipeline.hpp>


namespace clas12::algorithms {

using clas12::framework::Log;

Pi0ReconstructionAlg::Pi0ReconstructionAlg(std::string name, double pi0_mass_window)
    : SimpleAlgorithm(std::move(name))
    , m_pi0_mass_window{"pi0_mass_window", pi0_mass_window, "Pi0 mass window [GeV]"}
    , m_cutflow("Pi0 Reconstruction") {
    // Declare properties
    declare_property(m_min_photon_energy);
    declare_property(m_pi0_mass_window);
}

auto Pi0ReconstructionAlg::configure() -> void {
    // No services needed - pure reconstruction from event store data
}

auto Pi0ReconstructionAlg::select_photons(const std::vector<clas12::Particle>& photons) const -> std::vector<clas12::Particle> {
    std::vector<clas12::Particle> selected;
    selected.reserve(photons.size());

    for (const auto& photon : photons) {
        if (photon.E() >= m_min_photon_energy.get()) selected.push_back(photon);
    }

    return selected;
}

auto Pi0ReconstructionAlg::calculate_invariant_mass(const clas12::Particle& g1, const clas12::Particle& g2) -> double {
    return (g1.PxPyPzEVector() + g2.PxPyPzEVector()).M();
}

auto Pi0ReconstructionAlg::find_best_pi0(const std::vector<clas12::Particle>& photons) const -> std::optional<std::tuple<clas12::Particle, clas12::Particle, double>> {
    if (photons.size() < 2) return std::nullopt;

    std::optional<std::tuple<clas12::Particle, clas12::Particle, double>> best;
    double best_mass_diff = std::numeric_limits<double>::max();

    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            double mass = calculate_invariant_mass(photons[i], photons[j]);
            double mass_diff = std::abs(mass - PI0_MASS);
            if (mass_diff < m_pi0_mass_window.get() && mass_diff < best_mass_diff) {
                best_mass_diff = mass_diff;
                best = std::make_tuple(photons[i], photons[j], mass);
            }
        }
    }

    return best;
}

auto Pi0ReconstructionAlg::find_all_unique_pi0_candidates(const std::vector<clas12::Particle>& photons) const -> std::optional<std::vector<std::tuple<clas12::Particle, clas12::Particle, double>>> {

    if (photons.size() < 2) return std::nullopt;

    std::vector<std::tuple<clas12::Particle, clas12::Particle, double>> candidates;
    std::vector<bool> used(photons.size(), false);

    // Keep pairing until we can't find more valid pairs
    while (true) {
        double best_mass_diff = std::numeric_limits<double>::max();
        size_t best_i = 0, best_j = 0;
        bool found_pair = false;

        // Find the best remaining pair (closest to PI0_MASS)
        for (size_t i = 0; i < photons.size(); ++i) {
            if (used[i]) continue;

            for (size_t j = i + 1; j < photons.size(); ++j) {
                if (used[j]) continue;

                double mass = calculate_invariant_mass(photons[i], photons[j]);
                double mass_diff = std::abs(mass - PI0_MASS);

                if (mass_diff < m_pi0_mass_window.get() && mass_diff < best_mass_diff) {
                    best_mass_diff = mass_diff;
                    best_i = i;
                    best_j = j;
                    found_pair = true;
                }
            }
        }

        if (!found_pair) break;

        // Add the best pair and mark as used
        double mass = calculate_invariant_mass(photons[best_i], photons[best_j]);
        candidates.emplace_back(photons[best_i], photons[best_j], mass);
        used[best_i] = true;
        used[best_j] = true;
    }

    return candidates.empty() ? std::nullopt : std::make_optional(candidates);
}


auto Pi0ReconstructionAlg::on_initialize() -> clas12::framework::StatusCode {
    Log::subsection(fmt::format("{} Configuration", name()));
    Log::config(name(), "Pi0 mass window", fmt::format("±{} GeV", m_pi0_mass_window.get()));

    m_cutflow.add_cut("All Events");
    m_cutflow.add_cut("Has Electron");
    m_cutflow.add_cut("Has >= 2 Photons");
    m_cutflow.add_cut("Pi0 Found");

    Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

    return clas12::framework::StatusCode::SUCCESS();
}

auto Pi0ReconstructionAlg::process(clas12::framework::Event& event) -> clas12::framework::StatusCode {
    using namespace clas12::framework;

    // Read selected photons from the event store; enforce >= 2 for
    // a pair to exist before the pipeline even reaches the pair stage.
    auto photons_opt = event.retrieve_typed(Keys::SelectedPhotons_Pi0);
    const bool has_enough_photons = photons_opt && photons_opt->size() >= 2;

    // Eagerly run the upper-triangle pair enumeration that
    // `find_all_unique_pi0_candidates` used to contain, now expressed as
    // a PairPipeline over the photon collection. The filter bumps
    // "Pi0 Found" exactly once per surviving pair and `build<>` returns
    // the full CompositeParticle list for the publish stage.
    std::vector<clas12::physics::CompositeParticle> pi0_candidates;
    if (has_enough_photons) {
        pi0_candidates = unique_pairs(*photons_opt, m_cutflow)
            .filter("Pi0 Found",
                    [this](const clas12::Particle& g1, const clas12::Particle& g2) {
                        const double mass      = calculate_invariant_mass(g1, g2);
                        const double mass_diff = std::abs(mass - PI0_MASS);
                        return mass_diff < m_pi0_mass_window.get();
                    })
            .build([](const clas12::Particle& g1, const clas12::Particle& g2) {
                return clas12::physics::CompositeParticle{
                    std::vector<clas12::Particle>{g1, g2}, 111};
            });
    }

    return pipeline(event, m_cutflow)
        .count("All Events")
        .require(Decisions::HasGoodDISElectron)
        .count("Has Electron")
        .filter("Has >= 2 Photons", has_enough_photons)
        .filter("has pi0 candidate", !pi0_candidates.empty())
        .publish(Keys::Pi0Candidates, pi0_candidates)
        .set_decision(Decisions::HasPi0, true)
        .status();
}

auto Pi0ReconstructionAlg::on_finalize() -> clas12::framework::StatusCode {
    Log::subsection(fmt::format("{} Summary", name()));
    Log::blank();
    m_cutflow.print();
    Log::success(name(), "Finalization complete");

    return clas12::framework::StatusCode::SUCCESS();
}

}  // namespace clas12::algorithms
