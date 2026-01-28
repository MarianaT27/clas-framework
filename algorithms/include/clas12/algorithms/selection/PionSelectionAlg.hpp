#pragma once

#include <cmath>
#include <clas12/Constants.hpp>
#include <clas12/ElectronCuts.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>

namespace clas12::algorithms {

// Import Log singleton for convenient logging
using clas12::framework::Log;

/**
 * @brief N-Tuple data structure for pion kinematics
 */
struct PionNTupleData {
    double p = 0.0;
    double theta = 0.0;
    double phi = 0.0;
    double vz = 0.0;
    double chi2pid = 0.0;
    double beta = 0.0;
    double delta_t = 0.0;

    NTUPLE_FIELDS(p, theta, phi, vz, chi2pid, beta, delta_t)
};

/**
 * @brief Pion (π⁺/π⁻) selection and analysis algorithm
 *
 * Modernized using SimpleAlgorithm CRTP pattern with:
 * - Automatic service injection via require_service<T>() and optional_service<T>()
 * - Property system for configuration
 * - New histogram templates for physics quantities
 * - Reduced boilerplate through CRTP
 *
 * This algorithm demonstrates pion identification using:
 * - Chi2 PID cuts
 * - Momentum cuts
 * - Vertex Z cuts
 * - Time-of-flight (delta_t) cuts
 * - Beta cuts
 * - Cutflow tracking for efficiency
 */
class PionSelectionAlg : public clas12::framework::SimpleAlgorithm<PionSelectionAlg> {
   public:
    explicit PionSelectionAlg(std::string name = "PionSelection",
                              clas12::Target target = clas12::Target::LD2,
                              bool separate_charges = true)
        : SimpleAlgorithm(std::move(name))
        , m_target(target)
        , m_separate_charges(separate_charges)
        , m_cutflow_piplus("π⁺ Selection")
        , m_cutflow_piminus("π⁻ Selection") {
        // Declare configurable properties
        declare_property(m_chi2pid_min);
        declare_property(m_chi2pid_max);
        declare_property(m_momentum_min);
        declare_property(m_momentum_max);
        declare_property(m_enable_ntuple);
    }

    /**
     * @brief Configure required and optional services
     */
    auto configure() -> void override {
        // Required services
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");

        // Optional services - analysis continues even if not available
        m_log_svc = &optional_service<clas12::framework::LoggingService>("LoggingService");
        m_ntuple_svc = &optional_service<clas12::framework::NTupleService>("NTupleService");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Initializing pion selection algorithm");
        Log::info(name(), "Target: {}, Separate charges: {}",
                  m_target == clas12::Target::LD2 ? "LD2" : "Other",
                  m_separate_charges ? "yes" : "no");
        Log::info(name(), "Chi2PID range: [{}, {}], Momentum range: [{}, {}] GeV",
                  m_chi2pid_min.get(), m_chi2pid_max.get(),
                  m_momentum_min.get(), m_momentum_max.get());

        // Setup cutflows
        setup_cutflows();

        // Book histograms using new histogram templates
        book_histograms();

        // Setup N-tuple branches if service is available
        if (m_ntuple_svc && m_ntuple_svc->is_valid()) {
            setup_ntuples();
        } else {
            Log::warning(name(), "NTupleService not available - N-tuple output disabled");
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        // Only process pions if a good electron was found
        if (!event.passes_decision_typed(clas12::framework::Decisions::HasGoodDISElectron)) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Get banks using structured bindings
        auto& hipo_event = event.hipo_event();
        auto [REC_Event, REC_Particle, REC_Scintillator] =
            hipo_event.banks("REC::Event", "REC::Particle", "REC::Scintillator");

        if (REC_Event->rows() == 0) return clas12::framework::StatusCode::SUCCESS();

        double start_time = REC_Event->get<double>("startTime", 0);

        // Extract pions using ParticleSelectorService
        clas12::framework::ParticleMap particles = m_particle_svc->get()->extract_particles(
            *REC_Particle,
            {clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS});

        // Process π⁺ and collect selected pions
        const auto& pions_plus = clas12::framework::ParticleSelectorService::get_pions_plus(particles);
        auto selected_piplus = process_pions(pions_plus, *REC_Scintillator, start_time, true);

        // Process π⁻ and collect selected pions
        const auto& pions_minus = clas12::framework::ParticleSelectorService::get_pions_minus(particles);
        auto selected_piminus = process_pions(pions_minus, *REC_Scintillator, start_time, false);

        // Store selected pions in event for downstream algorithms
        event.store_typed(clas12::framework::Keys::SelectedPiPlus, selected_piplus);
        event.store_typed(clas12::framework::Keys::SelectedPiMinus, selected_piminus);

        // Set decision based on whether we have good pions
        event.set_decision_typed(clas12::framework::Decisions::HasGoodPiMinus, !selected_piminus.empty());
        event.set_decision_typed(clas12::framework::Decisions::HasGoodPiPlus, !selected_piplus.empty());

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Finalizing pion selection algorithm");

        auto piplus_count = m_cutflow_piplus.get_count("Delta-t");
        auto piminus_count = m_cutflow_piminus.get_count("Delta-t");

        Log::info(name(), "π⁺ passing all cuts: {}", piplus_count);
        Log::info(name(), "π⁻ passing all cuts: {}", piminus_count);

        if (m_ntuple_svc && m_ntuple_svc->is_valid()) {
            auto piplus_entries = m_ntuple_svc->get()->get_entries("pion_plus");
            auto piminus_entries = m_ntuple_svc->get()->get_entries("pion_minus");
            Log::info(name(), "N-tuple entries: π⁺={}, π⁻={}", piplus_entries, piminus_entries);
        }

        fmt::print("\n");
        m_cutflow_piplus.print();
        fmt::print("\n");
        m_cutflow_piminus.print();

        fmt::print("\nPion Analysis Statistics:\n");
        fmt::print("  π⁺ passing all cuts: {}\n", m_cutflow_piplus.get_count("Delta-t"));
        fmt::print("  π⁻ passing all cuts: {}\n", m_cutflow_piminus.get_count("Delta-t"));

        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::Target m_target;
    bool m_separate_charges;
    clas12::framework::Cutflow m_cutflow_piplus;
    clas12::framework::Cutflow m_cutflow_piminus;

    // Configurable properties
    clas12::framework::Property<double> m_chi2pid_min{"chi2pid_min", -10.0, "Minimum chi2PID"};
    clas12::framework::Property<double> m_chi2pid_max{"chi2pid_max", 10.0, "Maximum chi2PID"};
    clas12::framework::Property<double> m_momentum_min{"momentum_min", 0.3, "Minimum pion momentum [GeV]"};
    clas12::framework::Property<double> m_momentum_max{"momentum_max", 10.0, "Maximum pion momentum [GeV]"};
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Service handles (auto-injected via require_service/optional_service)
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::LoggingService>* m_log_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;

    // N-tuple writers (replace individual variables + manual registration)
    clas12::framework::NTupleWriter<PionNTupleData> m_ntuple_piplus;
    clas12::framework::NTupleWriter<PionNTupleData> m_ntuple_piminus;

    // Lock-free atomic histograms for π⁺
    struct {
        clas12::framework::AtomicHistogram1D* momentum = nullptr;
        clas12::framework::AtomicHistogram1D* theta = nullptr;
        clas12::framework::AtomicHistogram1D* phi = nullptr;
        clas12::framework::AtomicHistogram1D* vz = nullptr;
        clas12::framework::AtomicHistogram1D* chi2pid = nullptr;
        clas12::framework::AtomicHistogram1D* beta = nullptr;
        clas12::framework::AtomicHistogram2D* beta_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* delta_t_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* p_vs_theta = nullptr;
    } h_piplus;

    // Lock-free atomic histograms for π⁻
    struct {
        clas12::framework::AtomicHistogram1D* momentum = nullptr;
        clas12::framework::AtomicHistogram1D* theta = nullptr;
        clas12::framework::AtomicHistogram1D* phi = nullptr;
        clas12::framework::AtomicHistogram1D* vz = nullptr;
        clas12::framework::AtomicHistogram1D* chi2pid = nullptr;
        clas12::framework::AtomicHistogram1D* beta = nullptr;
        clas12::framework::AtomicHistogram2D* beta_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* delta_t_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* p_vs_theta = nullptr;
    } h_piminus;

    auto setup_cutflows() -> void {
        // π⁺ cutflow
        m_cutflow_piplus.add_cut("All Events");
        m_cutflow_piplus.add_cut("Has π⁺");
        m_cutflow_piplus.add_cut("Forward detector");
        m_cutflow_piplus.add_cut("Chi2 PID");
        m_cutflow_piplus.add_cut("Momentum");
        m_cutflow_piplus.add_cut("Vertex Z");
        m_cutflow_piplus.add_cut("Delta-t");

        // π⁻ cutflow
        m_cutflow_piminus.add_cut("All Events");
        m_cutflow_piminus.add_cut("Has π⁻");
        m_cutflow_piminus.add_cut("Forward detector");
        m_cutflow_piminus.add_cut("Chi2 PID");
        m_cutflow_piminus.add_cut("Momentum");
        m_cutflow_piminus.add_cut("Vertex Z");
        m_cutflow_piminus.add_cut("Delta-t");
    }

    auto book_histograms() -> void {
        auto* svc = m_hist_svc->get();

        // π⁺ histograms - all physics methods now use atomic histograms by default
        h_piplus.momentum = svc->book_momentum_hist("/Pion/PiPlus/momentum", "#pi^{+} Momentum");
        h_piplus.theta = svc->book_theta_hist("/Pion/PiPlus/theta", "#pi^{+} #theta");
        h_piplus.phi = svc->book_phi_hist("/Pion/PiPlus/phi", "#pi^{+} #phi");
        h_piplus.vz = svc->book_hist("/Pion/PiPlus/vz", "#pi^{+} v_{z};v_{z} [cm];Events", 200, -40, 40);
        h_piplus.chi2pid = svc->book_hist(
            "/Pion/PiPlus/chi2pid", "#pi^{+} #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);
        h_piplus.beta = svc->book_hist(
            "/Pion/PiPlus/beta", "#pi^{+} #beta;#beta;Events", 100, 0.7, 1.05);

        h_piplus.beta_vs_p = svc->book_hist2d(
            "/Pion/PiPlus/beta_vs_p", "#pi^{+} #beta vs p;p [GeV/c];#beta",
            100, 0, 10, 100, 0.7, 1.05);
        h_piplus.delta_t_vs_p = svc->book_hist2d(
            "/Pion/PiPlus/delta_t_vs_p", "#pi^{+} #Deltat vs p;p [GeV/c];#Deltat [ns]",
            100, 0, 10, 200, -5, 5);
        h_piplus.p_vs_theta = svc->book_hist2D("/Pion/PiPlus/p_vs_theta", "#pi^{+} p vs #theta", 200, 0, 50, 200, 0, 10);

        // π⁻ histograms - all physics methods now use atomic histograms by default
        h_piminus.momentum = svc->book_momentum_hist("/Pion/PiMinus/momentum", "#pi^{-} Momentum");
        h_piminus.theta = svc->book_theta_hist("/Pion/PiMinus/theta", "#pi^{-} #theta");
        h_piminus.phi = svc->book_phi_hist("/Pion/PiMinus/phi", "#pi^{-} #phi");
        h_piminus.vz = svc->book_hist("/Pion/PiMinus/vz", "#pi^{-} v_{z};v_{z} [cm];Events", 200, -40, 40);
        h_piminus.chi2pid = svc->book_hist(
            "/Pion/PiMinus/chi2pid", "#pi^{-} #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);
        h_piminus.beta = svc->book_hist(
            "/Pion/PiMinus/beta", "#pi^{-} #beta;#beta;Events", 100, 0.7, 1.05);

        h_piminus.beta_vs_p = svc->book_hist2d(
            "/Pion/PiMinus/beta_vs_p", "#pi^{-} #beta vs p;p [GeV/c];#beta",
            100, 0, 10, 100, 0.7, 1.05);
        h_piminus.delta_t_vs_p = svc->book_hist2d(
            "/Pion/PiMinus/delta_t_vs_p", "#pi^{-} #Deltat vs p;p [GeV/c];#Deltat [ns]",
            100, 0, 10, 200, -5, 5);
        h_piminus.p_vs_theta = svc->book_hist2D("/Pion/PiMinus/p_vs_theta", "#pi^{-} p vs #theta", 200, 0, 50, 200, 0, 10);
    }

    auto setup_ntuples() -> void {
        // Setup N-tuple using NTupleWriter
        m_ntuple_piplus.init(m_ntuple_svc->get(), "pion_plus");
        m_ntuple_piminus.init(m_ntuple_svc->get(), "pion_minus");

        Log::info(name(), "N-tuple branches registered for π⁺ and π⁻");
    }

    /**
     * @brief Process pions (either π⁺ or π⁻)
     * @return Vector of selected pions that pass all cuts
     */
    auto process_pions(const std::vector<clas12::Particle>& pions,
                       hipo::bank& REC_Scintillator,
                       double start_time,
                       bool is_piplus) -> std::vector<clas12::Particle> {
        std::vector<clas12::Particle> selected_pions;
        auto& cutflow = is_piplus ? m_cutflow_piplus : m_cutflow_piminus;
        cutflow.pass("All Events");

        if (pions.empty()) return selected_pions;

        // Process each pion
        for (const auto& pion : pions) {
            if (pion.pdg() != clas12::PIDs::PIPLUS && pion.pdg() != clas12::PIDs::PIMINUS) continue;  // Skip if PID does not match | should be useless
            cutflow.pass(is_piplus ? "Has π⁺" : "Has π⁻");

            // Forward detector cut
            if (pion.status() < 2000 || pion.status() > 4000) continue;
            cutflow.pass("Forward detector");


            // Chi2 PID cut
            if (!pass_chi2pid_cut(pion.chi2pid())) continue;
            cutflow.pass("Chi2 PID");

            // Momentum cut
            double p = pion.p();
            if (!pass_momentum_cut(p)) continue;
            cutflow.pass("Momentum");

            // Vertex Z cut
            if (!pass_vz_cut(pion.vz())) continue;
            cutflow.pass("Vertex Z");

            // Get scintillator info for delta_t
            clas12::ScintillatorBank scintillator = clas12::read_Scintillator_bank(
                REC_Scintillator, pion.index());

            // Calculate delta_t
            double delta_t = calculate_delta_t(pion, scintillator, start_time);

            // Delta_t cut
            if (!pass_delta_t_cut(p, delta_t)) continue;
            cutflow.pass("Delta-t");

            // Pion passed all cuts - add to selected list
            selected_pions.push_back(pion);

            // Fill histograms
            fill_histograms(pion, delta_t, is_piplus);
        }

        return selected_pions;
    }

    /**
     * @brief Calculate time-of-flight difference (delta_t)
     * delta_t = measured_time - expected_time
     */
    auto calculate_delta_t(const clas12::Particle& pion,
                           const clas12::ScintillatorBank& scintillator,
                           double start_time) -> double {
        double measured_time = scintillator.time - start_time;
        double path_length = scintillator.path;
        double p = pion.p();
        double m = clas12::Constants::PIMASS;

        // Calculate expected beta
        double beta_expected = p / std::sqrt(p * p + m * m);

        // Calculate expected time of flight
        double t_expected = path_length / (beta_expected * clas12::Constants::SPEED_OF_LIGHT);

        return measured_time - t_expected;
    }

    /**
     * @brief Third-order polynomial helper
     */
    static auto third_order_poly(double x, double a, double b, double c, double d) -> double {
        return a + b * x + c * x * x + d * x * x * x;
    }

    // Cut functions - using configurable properties
    auto pass_chi2pid_cut(double chi2pid) -> bool {
        return m_chi2pid_min.get() < chi2pid && chi2pid < m_chi2pid_max.get();
    }

    auto pass_momentum_cut(double p) -> bool {
        return p > m_momentum_min.get() && p < m_momentum_max.get();
    }

    auto pass_vz_cut(double vz) -> bool {
        // Use target-specific vertex cuts
        auto vz_range = clas12::ElectronCuts::get_vz_range(m_target);
        return vz_range.contains(vz);
    }

    /**
     * @brief Time-of-flight cut using momentum-dependent parameterization
     *
     * This uses a 3-sigma cut around a momentum-dependent mean with
     * parameterized sigma values from pion calibration studies.
     */
    auto pass_delta_t_cut(const double p, const double delta_t) -> bool {
        // Parameterization from pion timing studies
        const double popt_mean[4] = {-0.02872196, 0.00598219, 0.00158193, -0.00028493};
        const double popt_sigma[4] = {0.25132972, -0.09788015, 0.01734634, -0.00103597};

        double mean = third_order_poly(p, popt_mean[0], popt_mean[1], popt_mean[2], popt_mean[3]);
        double sigma = third_order_poly(p, popt_sigma[0], popt_sigma[1], popt_sigma[2], popt_sigma[3]);

        double cut_low = mean - 3.0 * sigma;
        double cut_high = mean + 3.0 * sigma;

        return cut_low < delta_t && delta_t < cut_high;
    }

    auto fill_histograms(const clas12::Particle& pion, double delta_t, bool is_piplus) -> void {
        double p = pion.p();
        double theta = pion.theta();
        double phi = pion.phi();
        double vz = pion.vz();
        double chi2pid = pion.chi2pid();
        double beta = pion.beta();

        if (is_piplus) {
            // Fill π⁺ histograms (lock-free atomic)
            h_piplus.momentum->fill(p);
            h_piplus.theta->fill(theta);
            h_piplus.phi->fill(phi);
            h_piplus.vz->fill(vz);
            h_piplus.chi2pid->fill(chi2pid);
            h_piplus.beta->fill(beta);
            h_piplus.beta_vs_p->fill(p, beta);
            h_piplus.delta_t_vs_p->fill(p, delta_t);
            h_piplus.p_vs_theta->fill(theta, p);

            // Fill π⁺ N-tuple using NTupleWriter
            if (m_ntuple_piplus) {
                m_ntuple_piplus.data().p = p;
                m_ntuple_piplus.data().theta = theta;
                m_ntuple_piplus.data().phi = phi;
                m_ntuple_piplus.data().vz = vz;
                m_ntuple_piplus.data().chi2pid = chi2pid;
                m_ntuple_piplus.data().beta = beta;
                m_ntuple_piplus.data().delta_t = delta_t;
                m_ntuple_piplus.fill();
            }
        } else {
            // Fill π⁻ histograms (lock-free atomic)
            h_piminus.momentum->fill(p);
            h_piminus.theta->fill(theta);
            h_piminus.phi->fill(phi);
            h_piminus.vz->fill(vz);
            h_piminus.chi2pid->fill(chi2pid);
            h_piminus.beta->fill(beta);
            h_piminus.beta_vs_p->fill(p, beta);
            h_piminus.delta_t_vs_p->fill(p, delta_t);
            h_piminus.p_vs_theta->fill(theta, p);

            // Fill π⁻ N-tuple using NTupleWriter
            if (m_ntuple_piminus) {
                m_ntuple_piminus.data().p = p;
                m_ntuple_piminus.data().theta = theta;
                m_ntuple_piminus.data().phi = phi;
                m_ntuple_piminus.data().vz = vz;
                m_ntuple_piminus.data().chi2pid = chi2pid;
                m_ntuple_piminus.data().beta = beta;
                m_ntuple_piminus.data().delta_t = delta_t;
                m_ntuple_piminus.fill();
            }
        }
    }
};

}  // namespace clas12::algorithms
