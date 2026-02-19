#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <Math/Vector4D.h>
#include <limits>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/ServiceHelpers.hpp>
#include "Math/GenVector/VectorUtil.h"


namespace clas12::algorithms {

using clas12::framework::Log;

Pi0ReconstructionAlg::Pi0ReconstructionAlg(std::string name, double min_photon_energy, double pi0_mass_window)
    : SimpleAlgorithm(std::move(name))
    , m_min_photon_energy{"min_photon_energy", min_photon_energy, "Minimum photon energy [GeV]"}
    , m_pi0_mass_window{"pi0_mass_window", pi0_mass_window, "Pi0 mass window [GeV]"}
    , m_cutflow("Pi0 Reconstruction") {
    // Declare properties
    declare_property(m_min_photon_energy);
    declare_property(m_pi0_mass_window);
    declare_property(m_enable_ntuple);
    // Declare required banks
    declare_bank("RUN::config");
    declare_bank("REC::Particle");
    declare_bank("REC::Calorimeter");
    declare_bank("REC::Scintillator");
}

auto Pi0ReconstructionAlg::configure() -> void {
    m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
    m_ntuple_svc = &require_service<clas12::framework::NTupleService>("NTupleService");
    m_photon_cuts_svc = &require_service<clas12::framework::PhotonCutsService>("PhotonCutsService");
}

auto Pi0ReconstructionAlg::select_photons(const std::vector<clas12::Particle>& photons) const -> std::vector<clas12::Particle> {
    std::vector<clas12::Particle> selected;
    selected.reserve(photons.size());

    for (const auto& photon : photons) {
        if (photon.E() >= m_min_photon_energy.get()) selected.push_back(photon);
    }

    return selected;
}

auto Pi0ReconstructionAlg::select_photons_with_AI(const hipo::bank_view& RUN_config, const hipo::bank_view& REC_Particle, const hipo::bank_view& REC_Calorimeter) const -> std::vector<clas12::Particle> {
    return m_photon_cuts_svc->get()->select_photons_with_AI(RUN_config, REC_Particle, REC_Calorimeter);
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
    Log::config(name(), "Min photon energy", fmt::format("{} GeV", m_min_photon_energy.get()));
    Log::config(name(), "Pi0 mass window", fmt::format("±{} GeV", m_pi0_mass_window.get()));
    Log::config(name(), "NTuple output", m_enable_ntuple.get() ? "enabled" : "disabled");

    book_histograms();

    if (m_enable_ntuple.get()) {
        clas12::framework::safe(m_ntuple_svc).with([this](auto*) { setup_ntuple(); });
    }

    m_cutflow.add_cut("All Events");
    m_cutflow.add_cut("Has Electron");
    m_cutflow.add_cut("Has >= 2 Photons");
    m_cutflow.add_cut("Pi0 Found");

    Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

    return clas12::framework::StatusCode::SUCCESS();
}

auto Pi0ReconstructionAlg::process(clas12::framework::Event& event) -> clas12::framework::StatusCode {
    m_cutflow.pass("All Events");

    // Check for good electron decision from upstream algorithm
    if (!event.passes_decision("HasGoodDISElectron")) return clas12::framework::StatusCode::FILTERED();
    m_cutflow.pass("Has Electron");

    const auto& photons = event.particles(clas12::PIDs::PHOTON);
    if (h_n_photons) h_n_photons->fill(static_cast<double>(photons.size()));
    for (const auto& photon : photons) {
        if (h_photon_energy_raw) h_photon_energy_raw->fill(photon.E());
        if (h_photon_theta_raw) h_photon_theta_raw->fill(photon.theta());
        if (h_photon_phi_raw) h_photon_phi_raw->fill(photon.phi());
    }

    const auto& RUN_config = event.get_bank("RUN::config");
    const auto& REC_Particle = event.get_bank("REC::Particle");
    const auto& REC_Calorimeter = event.get_bank("REC::Calorimeter");
    const auto& REC_Scintillator = event.get_bank("REC::Scintillator");

    auto selected_photons = select_photons_with_AI(RUN_config, REC_Particle, REC_Calorimeter);

    std::vector<clas12::Particle> fiducial_photons;
    for (const auto& photon : selected_photons) {
        clas12::CalorimeterBank calorimeter = clas12::read_Calorimeter_bank(REC_Calorimeter, photon.index());
        if (clas12::framework::PhotonCutsService::pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw)) {
            if (0.9 < photon.beta() && photon.beta() < 1.1) fiducial_photons.push_back(photon);
        }
    }
    selected_photons = fiducial_photons;

    for (const auto& photon : selected_photons) {
        if (h_photon_energy) h_photon_energy->fill(photon.E());
        if (h_photon_theta) h_photon_theta->fill(photon.theta());
    }

    if (selected_photons.size() < 2) return clas12::framework::StatusCode::FILTERED();
    m_cutflow.pass("Has >= 2 Photons");


    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            double mass = calculate_invariant_mass(photons[i], photons[j]);
            if (h_gg_mass_all) h_gg_mass_all->fill(mass);
        }
    }

    // Get photons with basic cuts applied
    const auto* electrons = event.get_particles<clas12::Particle>("SelectedElectrons");
    auto electron = (*electrons)[0];
    std::vector<clas12::Particle> traditional_photons;
    for (auto const& photon : photons) {
        if (photon.E() < 0.2) continue;
        if (ROOT::Math::VectorUtil::Angle(electron.PxPyPzVector(), photon.PxPyPzVector()) * 180.0 / M_PI < 8.0) continue;
        auto REC_Scintillator_bank = clas12::read_Scintillator_bank(REC_Scintillator, photon.index());
        double path_length = REC_Scintillator_bank.path;
        double beta = photon.beta();
        double delta_t = REC_Scintillator_bank.time - (path_length / (beta * clas12::Constants::SPEED_OF_LIGHT));
        if (delta_t < -1.67 || delta_t > 1.55) continue;
        traditional_photons.push_back(photon);
    }
    for (size_t i = 0; i < traditional_photons.size(); ++i) {
        for (size_t j = i + 1; j < traditional_photons.size(); ++j) {
            double mass = calculate_invariant_mass(traditional_photons[i], traditional_photons[j]);
            if (h_gg_mass_traditional) h_gg_mass_traditional->fill(mass);
        }
    } 

    for (size_t i = 0; i < selected_photons.size(); ++i) {
        for (size_t j = i + 1; j < selected_photons.size(); ++j) {
            double mass = calculate_invariant_mass(selected_photons[i], selected_photons[j]);
            if (h_gg_mass_AI) h_gg_mass_AI->fill(mass);
        }
    }

    auto candidates_opt = find_all_unique_pi0_candidates(selected_photons);
    if (!candidates_opt.has_value()) return clas12::framework::StatusCode::FILTERED();
    
    auto pi0_candidates = candidates_opt.value();
    m_cutflow.pass("Pi0 Found");

    // Use local vectors to avoid holding the lock during calculations
    std::vector<double> local_gs_E, local_gs_theta, local_gs_phi;
    std::vector<double> local_pi0_masses, local_pi0_ps, local_pi0_thetas, local_pi0_phis;
    double local_e_p{}, local_e_theta{}, local_e_phi{}, local_e_vz{};
    
    for (const auto& [g1, g2, pi0_mass] : pi0_candidates) {
        auto p1 = g1.PxPyPzEVector();
        auto p2 = g2.PxPyPzEVector();
        auto p_pi0 = p1 + p2;

        double pi0_p = std::sqrt(p_pi0.Px() * p_pi0.Px() + p_pi0.Py() * p_pi0.Py() + p_pi0.Pz() * p_pi0.Pz());
        double pi0_theta = std::acos(p_pi0.Pz() / pi0_p) * 180.0 / M_PI;
        double pi0_phi = std::atan2(p_pi0.Py(), p_pi0.Px()) * 180.0 / M_PI;

        // Calculate opening angle between photons
        auto p1_3 = g1.PxPyPzVector();
        auto p2_3 = g2.PxPyPzVector();
        double cos_angle = p1_3.Dot(p2_3) / (p1_3.R() * p2_3.R());
        double opening_angle = std::acos(cos_angle) * 180.0 / M_PI;

        // Fill thread-safe histograms
        if (h_gg_mass_selected) h_gg_mass_selected->fill(pi0_mass);
        if (h_pi0_momentum) h_pi0_momentum->fill(pi0_p);
        if (h_pi0_theta) h_pi0_theta->fill(pi0_theta);
        if (h_opening_angle) h_opening_angle->fill(opening_angle);

        // Fill local vectors
        local_gs_E.push_back(g1.E());
        local_gs_theta.push_back(g1.theta());
        local_gs_phi.push_back(g1.phi());
        local_gs_E.push_back(g2.E());
        local_gs_theta.push_back(g2.theta());
        local_gs_phi.push_back(g2.phi());
        local_pi0_masses.push_back(pi0_mass);
        local_pi0_ps.push_back(pi0_p);
        local_pi0_thetas.push_back(pi0_theta);
        local_pi0_phis.push_back(pi0_phi);
    }

    if (m_enable_ntuple.get() && !pi0_candidates.empty()) {
        clas12::framework::safe(m_ntuple_svc).with([&, this](auto* svc) {
            auto* electrons = event.get_particles<clas12::Particle>("SelectedElectrons");
            if (electrons && !electrons->empty()) {
                const auto& electron = (*electrons)[0];
                local_e_p = electron.p();
                local_e_theta = electron.theta();
                local_e_phi = electron.phi();
                local_e_vz = electron.vz();
            }

            // CRITICAL SECTION: Lock mutex before modifying shared vectors
            std::lock_guard<std::mutex> lock(m_ntuple_mutex);

            // Copy electron data
            m_e_p = local_e_p;
            m_e_theta = local_e_theta;
            m_e_phi = local_e_phi;
            m_e_vz = local_e_vz;

            // Copy photon data
            mgs_E = std::move(local_gs_E);
            mgs_theta = std::move(local_gs_theta);
            mgs_phi = std::move(local_gs_phi);

            // Copy pi0 data
            mpi0_masses = std::move(local_pi0_masses);
            mpi0_ps = std::move(local_pi0_ps);
            mpi0_thetas = std::move(local_pi0_thetas);
            mpi0_phis = std::move(local_pi0_phis);

            // Sanity check
            if (mgs_E.size() % 2 != 0) {
                Log::warning(name(), "Event has odd number of photons: {} candidates, {} photons",
                            pi0_candidates.size(), mgs_E.size());
            }

            // Fill the ntuple while holding the lock
            svc->fill("pi0_events");

            // Clear vectors for next event (still inside lock)
            mgs_E.clear();
            mgs_theta.clear();
            mgs_phi.clear();
            mpi0_masses.clear();
            mpi0_ps.clear();
            mpi0_thetas.clear();
            mpi0_phis.clear();
        });
    }

    return clas12::framework::StatusCode::SUCCESS();
}
auto Pi0ReconstructionAlg::on_finalize() -> clas12::framework::StatusCode {
    Log::subsection(fmt::format("{} Summary", name()));

    clas12::framework::safe(m_ntuple_svc).with([this](auto* svc) {
        auto entries = svc->get_entries("pi0_events");
        Log::stat(name(), "N-tuple entries", entries);
    });

    Log::blank();
    m_cutflow.print();
    Log::success(name(), "Finalization complete");

    return clas12::framework::StatusCode::SUCCESS();
}

auto Pi0ReconstructionAlg::book_histograms() -> void {
    clas12::framework::safe(m_hist_svc).with([this](auto* svc) {
        // Book lock-free atomic histograms for parallel event processing
        h_photon_energy_raw = svc->book_atomic_hist1D("/Pi0/photon_energy_raw", "Raw Photon Energy;E [GeV];Events", 100, 0, 5);
        h_photon_theta_raw = svc->book_atomic_hist1D("/Pi0/photon_theta_raw", "Raw Photon Theta;#theta [deg];Events", 180, 0, 40);
        h_photon_phi_raw = svc->book_atomic_phi_hist("/Pi0/photon_phi_raw", "Raw Photon Phi");

        h_photon_energy = svc->book_atomic_hist1D("/Pi0/photon_energy", "Photon Energy;E [GeV];Events", 100, 0, 5);
        h_photon_theta = svc->book_atomic_theta_hist("/Pi0/photon_theta", "Photon Theta");
        h_n_photons = svc->book_atomic_hist1D("/Pi0/n_photons", "Number of Photons;N_{#gamma};Events", 20, 0, 20);
        h_gg_mass_all = svc->book_atomic_hist1D("/Pi0/gg_mass_all", "All #gamma#gamma Invariant Mass;M_{#gamma#gamma} [GeV/c^{2}];Events", 200, 0, 0.5);
        h_gg_mass_traditional = svc->book_atomic_hist1D("/Pi0/gg_mass_traditional", "Traditional #gamma#gamma Invariant Mass;M_{#gamma#gamma} [GeV/c^{2}];Events", 200, 0, 0.5);
        h_gg_mass_AI = svc->book_atomic_hist1D("/Pi0/gg_mass_AI", "AI-selected #gamma#gamma Invariant Mass;M_{#gamma#gamma} [GeV/c^{2}];Events", 200, 0, 0.5);
        h_gg_mass_selected = svc->book_atomic_hist1D("/Pi0/gg_mass_selected", "Selected #pi^{0} Candidate Mass;M_{#gamma#gamma} [GeV/c^{2}];Events", 200, 0.0, 0.35);
        h_pi0_momentum = svc->book_atomic_momentum_hist("/Pi0/pi0_momentum", "#pi^{0} Momentum");
        h_pi0_theta = svc->book_atomic_theta_hist("/Pi0/pi0_theta", "#pi^{0} Theta");
        h_opening_angle = svc->book_atomic_theta_hist("/Pi0/opening_angle", "Photon Opening Angle");
    });
}

auto Pi0ReconstructionAlg::setup_ntuple() -> void {
    if (!m_ntuple_svc || !m_ntuple_svc->is_valid()) return;
    auto* svc = m_ntuple_svc->get();
    
    // Electron kinematics
    svc->register_branch("pi0_events", "e_p", &m_e_p);
    svc->register_branch("pi0_events", "e_theta", &m_e_theta);
    svc->register_branch("pi0_events", "e_phi", &m_e_phi);
    svc->register_branch("pi0_events", "e_vz", &m_e_vz);

    // Photon kinematics
    svc->register_branch("pi0_events", "mgs_E", &mgs_E);
    svc->register_branch("pi0_events", "mgs_theta", &mgs_theta);
    svc->register_branch("pi0_events", "mgs_phi", &mgs_phi);
    
    // Pi0 candidate properties
    svc->register_branch("pi0_events", "mpi0_masses", &mpi0_masses);
    svc->register_branch("pi0_events", "mpi0_ps", &mpi0_ps);
    svc->register_branch("pi0_events", "mpi0_thetas", &mpi0_thetas);
    svc->register_branch("pi0_events", "mpi0_phis", &mpi0_phis);

    Log::info(name(), "N-tuple branches registered for pi0_events");
}

}  // namespace clas12::algorithms
