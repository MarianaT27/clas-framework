#include "Algorithms/Pi0ReconstructionAlg.hpp"
#include <Math/Vector4D.h>
#include <limits>

namespace Algorithms {

Pi0ReconstructionAlg::Pi0ReconstructionAlg(std::string name, double min_photon_energy, double pi0_mass_window)
    : Algorithm(std::move(name)), m_min_photon_energy(min_photon_energy), m_pi0_mass_window(pi0_mass_window), m_cutflow("Pi0 Reconstruction") {}

auto Pi0ReconstructionAlg::select_photons(const std::vector<Core::Particle>& photons) const -> std::vector<Core::Particle> {
    std::vector<Core::Particle> selected;
    selected.reserve(photons.size());

    for (const auto& photon : photons) {
        if (photon.E() >= m_min_photon_energy) selected.push_back(photon);
    }

    return selected;
}

auto Pi0ReconstructionAlg::select_photons_with_AI(hipo::bank& RUN_config, hipo::bank& REC_Particle, hipo::bank& REC_Calorimeter) const -> std::vector<Core::Particle> {
    return Core::PhotonCuts::select_photons_with_AI(RUN_config, REC_Particle, REC_Calorimeter, 2);
}

auto Pi0ReconstructionAlg::calculate_invariant_mass(const Core::Particle& g1, const Core::Particle& g2) -> double {
    return (g1.PxPyPzEVector() + g2.PxPyPzEVector()).M();
}

auto Pi0ReconstructionAlg::find_best_pi0(const std::vector<Core::Particle>& photons) const -> std::optional<std::tuple<Core::Particle, Core::Particle, double>> {
    if (photons.size() < 2) return std::nullopt;

    std::optional<std::tuple<Core::Particle, Core::Particle, double>> best;
    double best_mass_diff = std::numeric_limits<double>::max();

    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            double mass = calculate_invariant_mass(photons[i], photons[j]);
            double mass_diff = std::abs(mass - PI0_MASS);
            if (mass_diff < m_pi0_mass_window && mass_diff < best_mass_diff) {
                best_mass_diff = mass_diff;
                best = std::make_tuple(photons[i], photons[j], mass);
            }
        }
    }

    return best;
}

auto Pi0ReconstructionAlg::initialize() -> Framework::StatusCode {
    auto log_sc = initialize_handle(m_log_svc);
    if (log_sc.is_failure()) fmt::print(stderr, "WARNING: LoggingService not available\n");

    if (m_log_svc) {
        LOG_INFO(m_log_svc.get(), "Initializing Pi0ReconstructionAlg");
        LOG_INFO(m_log_svc.get(), "  Min photon energy: {} GeV", m_min_photon_energy);
        LOG_INFO(m_log_svc.get(), "  Pi0 mass window: ±{} GeV", m_pi0_mass_window);
    }

    auto hist_sc = initialize_handle(m_hist_svc);
    if (hist_sc.is_failure()) {
        if (m_log_svc) LOG_ERROR(m_log_svc.get(), "Could not get HistogramService: {}", hist_sc.message());
        fmt::print(stderr, "ERROR: {}\n", hist_sc.message());
        return hist_sc;
    }

    auto ntuple_sc = initialize_handle(m_ntuple_svc);
    if (ntuple_sc.is_failure()) {
        if (m_log_svc) LOG_WARNING(m_log_svc.get(), "Could not get NTupleService - N-tuple output disabled");
    }

    book_histograms();

    if (m_ntuple_svc) setup_ntuple();

    m_cutflow.add_cut("All Events");
    m_cutflow.add_cut("Has Electron");
    m_cutflow.add_cut("Has >= 2 Photons");
    m_cutflow.add_cut("Pi0 Found");

    // Inject logger into cutflow for structured output
    if (m_log_svc) {
        m_cutflow.set_logger(m_log_svc.get());
        LOG_INFO(m_log_svc.get(), "Initialized with 4 cuts in cutflow");
    }

    return Framework::StatusCode::SUCCESS();
}

auto Pi0ReconstructionAlg::execute(Framework::Event& event) -> Framework::StatusCode {
    m_cutflow.pass("All Events");

    // Check for good electron decision from upstream algorithm
    if (!event.passes_decision("HasGoodElectron")) return Framework::StatusCode::SUCCESS();
    m_cutflow.pass("Has Electron");

    const auto& photons = event.particles(Core::PIDs::PHOTON);
    if (h_n_photons) h_n_photons->Fill(static_cast<double>(photons.size()));

    auto& hipo_event = event.hipo_event();
    hipo::bank& RUN_config = hipo_event.get_bank("RUN::config");
    hipo::bank& REC_Particle = hipo_event.get_bank("REC::Particle");
    hipo::bank& REC_Calorimeter = hipo_event.get_bank("REC::Calorimeter");

    auto selected_photons = select_photons_with_AI(RUN_config, REC_Particle, REC_Calorimeter);

    std::vector<Core::Particle> fiducial_photons;
    for (const auto& photon : selected_photons) {
        Core::CalorimeterBank calorimeter = Core::read_Calorimeter_bank(REC_Calorimeter, photon.index());
        if (Core::PhotonCuts::pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw)) fiducial_photons.push_back(photon);
    }
    selected_photons = fiducial_photons;

    for (const auto& photon : selected_photons) {
        if (h_photon_energy) h_photon_energy->Fill(photon.E());
        if (h_photon_theta) h_photon_theta->Fill(photon.theta());
    }

    if (selected_photons.size() < 2) return Framework::StatusCode::SUCCESS();
    m_cutflow.pass("Has >= 2 Photons");

    for (size_t i = 0; i < selected_photons.size(); ++i) {
        for (size_t j = i + 1; j < selected_photons.size(); ++j) {
            double mass = calculate_invariant_mass(selected_photons[i], selected_photons[j]);
            if (h_gg_mass_all) h_gg_mass_all->Fill(mass);
        }
    }

    // Find best π⁰ candidate
    auto pi0_candidate = find_best_pi0(selected_photons);

    if (!pi0_candidate.has_value()) return Framework::StatusCode::SUCCESS();
    m_cutflow.pass("Pi0 Found");

    const auto& [g1, g2, pi0_mass] = pi0_candidate.value();

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

    // Fill histograms
    if (h_gg_mass_selected) h_gg_mass_selected->Fill(pi0_mass);
    if (h_pi0_momentum) h_pi0_momentum->Fill(pi0_p);
    if (h_pi0_theta) h_pi0_theta->Fill(pi0_theta);
    if (h_opening_angle) h_opening_angle->Fill(opening_angle);

    // Fill N-tuple if service is available
    if (m_ntuple_svc) {
        // Get electron from event store (stored by ElectronSelectionAlg)
        auto* electrons = event.get_particles<Core::Particle>("SelectedElectrons");
        if (electrons && !electrons->empty()) {
            const auto& electron = (*electrons)[0];
            m_e_p = electron.p();
            m_e_theta = electron.theta();
            m_e_phi = electron.phi();
            m_e_vz = electron.vz();
        }

        // Photon 1
        m_g1_E = g1.E();
        m_g1_theta = g1.theta();
        m_g1_phi = g1.phi();

        // Photon 2
        m_g2_E = g2.E();
        m_g2_theta = g2.theta();
        m_g2_phi = g2.phi();

        // π⁰
        m_pi0_mass = pi0_mass;
        m_pi0_p = pi0_p;
        m_pi0_theta = pi0_theta;
        m_pi0_phi = pi0_phi;

        m_ntuple_svc->fill("pi0_events");
    }

    return Framework::StatusCode::SUCCESS();
}

auto Pi0ReconstructionAlg::finalize() -> Framework::StatusCode {
    if (m_log_svc) {
        LOG_INFO(m_log_svc.get(), "Finalizing Pi0ReconstructionAlg");
        if (m_ntuple_svc) {
            auto entries = m_ntuple_svc->get_entries("pi0_events");
            LOG_INFO(m_log_svc.get(), "N-tuple has {} entries", entries);
        }
    }

    fmt::print("\n");
    m_cutflow.print();

    return Framework::StatusCode::SUCCESS();
}

auto Pi0ReconstructionAlg::book_histograms() -> void {
    h_photon_energy = m_hist_svc->book_hist1D("/Pi0/photon_energy", "Photon Energy;E [GeV];Events", 100, 0, 5);
    h_photon_theta = m_hist_svc->book_hist1D("/Pi0/photon_theta", "Photon Theta;#theta [deg];Events", 180, 0, 180);
    h_n_photons = m_hist_svc->book_hist1D("/Pi0/n_photons", "Number of Photons;N_{#gamma};Events", 20, 0, 20);
    h_gg_mass_all = m_hist_svc->book_hist1D("/Pi0/gg_mass_all", "All #gamma#gamma Invariant Mass;M_{#gamma#gamma} [GeV/c^{2}];Events", 200, 0, 0.5);
    h_gg_mass_selected = m_hist_svc->book_hist1D("/Pi0/gg_mass_selected", "Selected #pi^{0} Candidate Mass;M_{#gamma#gamma} [GeV/c^{2}];Events", 200, 0.0, 0.35);
    h_pi0_momentum = m_hist_svc->book_hist1D("/Pi0/pi0_momentum", "#pi^{0} Momentum;p [GeV/c];Events", 100, 0, 10);
    h_pi0_theta = m_hist_svc->book_hist1D("/Pi0/pi0_theta", "#pi^{0} Theta;#theta [deg];Events", 180, 0, 180);
    h_opening_angle = m_hist_svc->book_hist1D("/Pi0/opening_angle", "Photon Opening Angle;#theta_{#gamma#gamma} [deg];Events", 180, 0, 180);
}

auto Pi0ReconstructionAlg::setup_ntuple() -> void {
    // Electron kinematics
    m_ntuple_svc->register_branch("pi0_events", "e_p", &m_e_p);
    m_ntuple_svc->register_branch("pi0_events", "e_theta", &m_e_theta);
    m_ntuple_svc->register_branch("pi0_events", "e_phi", &m_e_phi);
    m_ntuple_svc->register_branch("pi0_events", "e_vz", &m_e_vz);

    // Photon 1 kinematics
    m_ntuple_svc->register_branch("pi0_events", "g1_E", &m_g1_E);
    m_ntuple_svc->register_branch("pi0_events", "g1_theta", &m_g1_theta);
    m_ntuple_svc->register_branch("pi0_events", "g1_phi", &m_g1_phi);

    // Photon 2 kinematics
    m_ntuple_svc->register_branch("pi0_events", "g2_E", &m_g2_E);
    m_ntuple_svc->register_branch("pi0_events", "g2_theta", &m_g2_theta);
    m_ntuple_svc->register_branch("pi0_events", "g2_phi", &m_g2_phi);

    // π⁰ kinematics
    m_ntuple_svc->register_branch("pi0_events", "pi0_mass", &m_pi0_mass);
    m_ntuple_svc->register_branch("pi0_events", "pi0_p", &m_pi0_p);
    m_ntuple_svc->register_branch("pi0_events", "pi0_theta", &m_pi0_theta);
    m_ntuple_svc->register_branch("pi0_events", "pi0_phi", &m_pi0_phi);

    if (m_log_svc) LOG_INFO(m_log_svc.get(), "N-tuple branches registered for pi0_events");
}

}  // namespace Algorithms
