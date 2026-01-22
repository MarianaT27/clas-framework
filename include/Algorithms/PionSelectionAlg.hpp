#pragma once

#include <cmath>
#include <memory>
#include "CLAS/Constants.hpp"
#include "CLAS/ElectronCuts.hpp"
#include "CLAS/ReadBank.hpp"
#include "CLAS/Target.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "TH1D.h"
#include "TH2D.h"

namespace Algorithms {

// Import Log singleton for convenient logging
using Framework::Log;

/**
 * @brief Pion (π⁺/π⁻) selection and analysis algorithm
 *
 * This algorithm demonstrates pion identification using:
 * - Chi2 PID cuts
 * - Momentum cuts
 * - Vertex Z cuts
 * - Time-of-flight (delta_t) cuts
 * - Beta cuts
 * - Cutflow tracking for efficiency
 */
class PionSelectionAlg : public Framework::Algorithm {
   public:
    explicit PionSelectionAlg(std::string name = "PionSelection",
                              CLAS::Target target = CLAS::Target::LD2,
                              bool separate_charges = true)
        : Algorithm(std::move(name)), m_target(target), m_separate_charges(separate_charges), m_cutflow_piplus("π⁺ Selection"), m_cutflow_piminus("π⁻ Selection") {}

    auto initialize() -> Framework::StatusCode override {
        // Get services
        m_log_svc = service<Framework::LoggingService>("LoggingService");
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        m_ntuple_svc = service<Framework::NTupleService>("NTupleService");
        m_particle_svc = service<Framework::ParticleSelectorService>("ParticleSelectorService");

        if (Log::is_available()) {
            Log::info(name(), "Initializing pion selection algorithm");
            Log::info(name(), "Target: {}, Separate charges: {}",
                      m_target == CLAS::Target::LD2 ? "LD2" : "Other",
                      m_separate_charges ? "yes" : "no");
        }

        if (!m_hist_svc) {
            Log::error(name(), "Could not get HistogramService");
            return Framework::StatusCode::FAILURE();
        }

        // Setup cutflows
        setup_cutflows();

        // Book histograms
        book_histograms();

        // Setup N-tuple branches
        if (m_ntuple_svc) {
            setup_ntuples();
        } else {
            Log::warning(name(), "NTupleService not available - N-tuple output disabled");
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        // Only process pions if a good electron was found
        if (!event.passes_decision("HasGoodElectron")) {
            return Framework::StatusCode::FILTERED();
        }

        // Get banks using structured bindings
        auto& hipo_event = event.hipo_event();
        auto [REC_Event, REC_Particle, REC_Scintillator] =
            hipo_event.banks("REC::Event", "REC::Particle", "REC::Scintillator");

        if (REC_Event->rows() == 0) return Framework::StatusCode::SUCCESS();

        double start_time = REC_Event->get<double>("startTime", 0);

        // Extract pions using ParticleSelectorService
        Framework::ParticleMap particles = m_particle_svc->extract_particles(
            *REC_Particle,
            {CLAS::PIDs::PIPLUS, CLAS::PIDs::PIMINUS});

        // Process π⁺ and collect selected pions
        const auto& pions_plus = m_particle_svc->get_pions_plus(particles);
        auto selected_piplus = process_pions(pions_plus, *REC_Scintillator, start_time, true);

        // Process π⁻ and collect selected pions
        const auto& pions_minus = m_particle_svc->get_pions_minus(particles);
        auto selected_piminus = process_pions(pions_minus, *REC_Scintillator, start_time, false);

        // Store selected pions in event for downstream algorithms
        event.store_particles("SelectedPiPlus", selected_piplus);
        event.store_particles("SelectedPiMinus", selected_piminus);

        // Set decision based on whether we have good pions
        bool has_good_pions = !selected_piplus.empty() && !selected_piminus.empty();
        event.set_decision("HasGoodPions", has_good_pions);

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        if (Log::is_available()) {
            Log::info(name(), "Finalizing pion selection algorithm");

            auto piplus_count = m_cutflow_piplus.get_count("Delta-t");
            auto piminus_count = m_cutflow_piminus.get_count("Delta-t");

            Log::info(name(), "π⁺ passing all cuts: {}", piplus_count);
            Log::info(name(), "π⁻ passing all cuts: {}", piminus_count);

            if (m_ntuple_svc) {
                auto piplus_entries = m_ntuple_svc->get_entries("pion_plus");
                auto piminus_entries = m_ntuple_svc->get_entries("pion_minus");
                Log::info(name(), "N-tuple entries: π⁺={}, π⁻={}", piplus_entries, piminus_entries);
            }
        }

        fmt::print("\n");
        m_cutflow_piplus.print();
        fmt::print("\n");
        m_cutflow_piminus.print();

        fmt::print("\nPion Analysis Statistics:\n");
        fmt::print("  π⁺ passing all cuts: {}\n", m_cutflow_piplus.get_count("Delta-t"));
        fmt::print("  π⁻ passing all cuts: {}\n", m_cutflow_piminus.get_count("Delta-t"));

        return Framework::StatusCode::SUCCESS();
    }

   private:
    CLAS::Target m_target;
    bool m_separate_charges;
    Framework::Cutflow m_cutflow_piplus;
    Framework::Cutflow m_cutflow_piminus;

    // Services
    Framework::LoggingService* m_log_svc = nullptr;
    Framework::HistogramService* m_hist_svc = nullptr;
    Framework::NTupleService* m_ntuple_svc = nullptr;
    Framework::ParticleSelectorService* m_particle_svc = nullptr;

    // N-tuple variables for π⁺
    double m_nt_piplus_p = 0.0;
    double m_nt_piplus_theta = 0.0;
    double m_nt_piplus_phi = 0.0;
    double m_nt_piplus_vz = 0.0;
    double m_nt_piplus_chi2pid = 0.0;
    double m_nt_piplus_beta = 0.0;
    double m_nt_piplus_delta_t = 0.0;

    // N-tuple variables for π⁻
    double m_nt_piminus_p = 0.0;
    double m_nt_piminus_theta = 0.0;
    double m_nt_piminus_phi = 0.0;
    double m_nt_piminus_vz = 0.0;
    double m_nt_piminus_chi2pid = 0.0;
    double m_nt_piminus_beta = 0.0;
    double m_nt_piminus_delta_t = 0.0;

    // Histograms for π⁺
    struct {
        TH1D* momentum = nullptr;
        TH1D* theta = nullptr;
        TH1D* phi = nullptr;
        TH1D* vz = nullptr;
        TH1D* chi2pid = nullptr;
        TH1D* beta = nullptr;
        TH2D* beta_vs_p = nullptr;
        TH2D* delta_t_vs_p = nullptr;
        TH2D* p_vs_theta = nullptr;
    } h_piplus;

    // Histograms for π⁻
    struct {
        TH1D* momentum = nullptr;
        TH1D* theta = nullptr;
        TH1D* phi = nullptr;
        TH1D* vz = nullptr;
        TH1D* chi2pid = nullptr;
        TH1D* beta = nullptr;
        TH2D* beta_vs_p = nullptr;
        TH2D* delta_t_vs_p = nullptr;
        TH2D* p_vs_theta = nullptr;
    } h_piminus;

    auto setup_cutflows() -> void {
        // π⁺ cutflow
        m_cutflow_piplus.add_cut("All Events");
        m_cutflow_piplus.add_cut("Has π⁺");
        m_cutflow_piplus.add_cut("Forward detector");
        m_cutflow_piplus.add_cut("Chi2 PID");
        m_cutflow_piplus.add_cut("Momentum");
        m_cutflow_piplus.add_cut("Vertex Z");
        m_cutflow_piplus.add_cut("Beta");
        m_cutflow_piplus.add_cut("Delta-t");

        // π⁻ cutflow
        m_cutflow_piminus.add_cut("All Events");
        m_cutflow_piminus.add_cut("Has π⁻");
        m_cutflow_piminus.add_cut("Forward detector");
        m_cutflow_piminus.add_cut("Chi2 PID");
        m_cutflow_piminus.add_cut("Momentum");
        m_cutflow_piminus.add_cut("Vertex Z");
        m_cutflow_piminus.add_cut("Beta");
        m_cutflow_piminus.add_cut("Delta-t");
    }

    auto book_histograms() -> void {
        // π⁺ histograms
        h_piplus.momentum = m_hist_svc->book_hist1D(
            "/Pion/PiPlus/momentum", "π⁺ Momentum;p [GeV/c];Events", 100, 0, 10);
        h_piplus.theta = m_hist_svc->book_hist1D(
            "/Pion/PiPlus/theta", "π⁺ #theta;#theta [deg];Events", 180, 0, 180);
        h_piplus.phi = m_hist_svc->book_hist1D(
            "/Pion/PiPlus/phi", "π⁺ #phi;#phi [deg];Events", 180, -180, 180);
        h_piplus.vz = m_hist_svc->book_hist1D(
            "/Pion/PiPlus/vz", "π⁺ v_{z};v_{z} [cm];Events", 200, -20, 20);
        h_piplus.chi2pid = m_hist_svc->book_hist1D(
            "/Pion/PiPlus/chi2pid", "π⁺ #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);
        h_piplus.beta = m_hist_svc->book_hist1D(
            "/Pion/PiPlus/beta", "π⁺ #beta;#beta;Events", 100, 0, 1.2);

        h_piplus.beta_vs_p = m_hist_svc->book_hist2D(
            "/Pion/PiPlus/beta_vs_p", "π⁺ #beta vs p;p [GeV/c];#beta",
            100, 0, 10, 100, 0, 1.2);
        h_piplus.delta_t_vs_p = m_hist_svc->book_hist2D(
            "/Pion/PiPlus/delta_t_vs_p", "π⁺ #Deltat vs p;p [GeV/c];#Deltat [ns]",
            100, 0, 10, 200, -5, 5);
        h_piplus.p_vs_theta = m_hist_svc->book_hist2D(
            "/Pion/PiPlus/p_vs_theta", "π⁺ p vs #theta;#theta [deg];p [GeV/c]",
            180, 0, 180, 100, 0, 10);

        // π⁻ histograms
        h_piminus.momentum = m_hist_svc->book_hist1D(
            "/Pion/PiMinus/momentum", "π⁻ Momentum;p [GeV/c];Events", 100, 0, 10);
        h_piminus.theta = m_hist_svc->book_hist1D(
            "/Pion/PiMinus/theta", "π⁻ #theta;#theta [deg];Events", 180, 0, 180);
        h_piminus.phi = m_hist_svc->book_hist1D(
            "/Pion/PiMinus/phi", "π⁻ #phi;#phi [deg];Events", 180, -180, 180);
        h_piminus.vz = m_hist_svc->book_hist1D(
            "/Pion/PiMinus/vz", "π⁻ v_{z};v_{z} [cm];Events", 200, -20, 20);
        h_piminus.chi2pid = m_hist_svc->book_hist1D(
            "/Pion/PiMinus/chi2pid", "π⁻ #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);
        h_piminus.beta = m_hist_svc->book_hist1D(
            "/Pion/PiMinus/beta", "π⁻ #beta;#beta;Events", 100, 0, 1.2);

        h_piminus.beta_vs_p = m_hist_svc->book_hist2D(
            "/Pion/PiMinus/beta_vs_p", "π⁻ #beta vs p;p [GeV/c];#beta",
            100, 0, 10, 100, 0, 1.2);
        h_piminus.delta_t_vs_p = m_hist_svc->book_hist2D(
            "/Pion/PiMinus/delta_t_vs_p", "π⁻ #Deltat vs p;p [GeV/c];#Deltat [ns]",
            100, 0, 10, 200, -5, 5);
        h_piminus.p_vs_theta = m_hist_svc->book_hist2D(
            "/Pion/PiMinus/p_vs_theta", "π⁻ p vs #theta;#theta [deg];p [GeV/c]",
            180, 0, 180, 100, 0, 10);
    }

    auto setup_ntuples() -> void {
        // Setup N-tuple for π⁺
        m_ntuple_svc->register_branch("pion_plus", "p", &m_nt_piplus_p);
        m_ntuple_svc->register_branch("pion_plus", "theta", &m_nt_piplus_theta);
        m_ntuple_svc->register_branch("pion_plus", "phi", &m_nt_piplus_phi);
        m_ntuple_svc->register_branch("pion_plus", "vz", &m_nt_piplus_vz);
        m_ntuple_svc->register_branch("pion_plus", "chi2pid", &m_nt_piplus_chi2pid);
        m_ntuple_svc->register_branch("pion_plus", "beta", &m_nt_piplus_beta);
        m_ntuple_svc->register_branch("pion_plus", "delta_t", &m_nt_piplus_delta_t);

        // Setup N-tuple for π⁻
        m_ntuple_svc->register_branch("pion_minus", "p", &m_nt_piminus_p);
        m_ntuple_svc->register_branch("pion_minus", "theta", &m_nt_piminus_theta);
        m_ntuple_svc->register_branch("pion_minus", "phi", &m_nt_piminus_phi);
        m_ntuple_svc->register_branch("pion_minus", "vz", &m_nt_piminus_vz);
        m_ntuple_svc->register_branch("pion_minus", "chi2pid", &m_nt_piminus_chi2pid);
        m_ntuple_svc->register_branch("pion_minus", "beta", &m_nt_piminus_beta);
        m_ntuple_svc->register_branch("pion_minus", "delta_t", &m_nt_piminus_delta_t);

        if (Log::is_available()) {
            Log::info(name(), "N-tuple branches registered for π⁺ and π⁻");
        }
    }

    /**
     * @brief Process pions (either π⁺ or π⁻)
     * @return Vector of selected pions that pass all cuts
     */
    auto process_pions(const std::vector<CLAS::Particle>& pions,
                       hipo::bank& REC_Scintillator,
                       double start_time,
                       bool is_piplus) -> std::vector<CLAS::Particle> {
        std::vector<CLAS::Particle> selected_pions;
        auto& cutflow = is_piplus ? m_cutflow_piplus : m_cutflow_piminus;
        cutflow.pass("All Events");

        if (pions.empty()) return selected_pions;

        // Process each pion
        for (const auto& pion : pions) {
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

            // Beta cut
            double beta = pion.beta();
            if (!pass_beta_cut(beta, p)) continue;
            cutflow.pass("Beta");

            // Get scintillator info for delta_t
            CLAS::ScintillatorBank scintillator = CLAS::read_Scintillator_bank(
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
    auto calculate_delta_t(const CLAS::Particle& pion,
                           const CLAS::ScintillatorBank& scintillator,
                           double start_time) -> double {
        double measured_time = scintillator.time - start_time;
        double path_length = scintillator.path;
        double p = pion.p();
        double m = CLAS::Constants::PIMASS;

        // Calculate expected beta
        double beta_expected = p / std::sqrt(p * p + m * m);

        // Calculate expected time of flight
        double t_expected = path_length / (beta_expected * CLAS::Constants::SPEED_OF_LIGHT);

        return measured_time - t_expected;
    }

    /**
     * @brief Third-order polynomial helper
     */
    static auto third_order_poly(double x, double a, double b, double c, double d) -> double {
        return a + b * x + c * x * x + d * x * x * x;
    }

    // Cut functions
    auto pass_chi2pid_cut(double chi2pid) -> bool {
        return -3.0 < chi2pid && chi2pid < 3.0;
    }

    auto pass_momentum_cut(double p) -> bool {
        return p > 0.3 && p < 10.0;  // Reasonable range for pions
    }

    auto pass_vz_cut(double vz) -> bool {
        // Use target-specific vertex cuts
        auto vz_range = CLAS::ElectronCuts::get_vz_range(m_target);
        return vz_range.contains(vz);
    }

    auto pass_beta_cut(double beta, double p) -> bool {
        // Basic beta cut - could be refined
        double m = CLAS::Constants::PIMASS;
        double beta_expected = p / std::sqrt(p * p + m * m);
        double delta_beta = std::abs(beta - beta_expected);
        return delta_beta < 0.05;  // 5% tolerance
    }

    /**
     * @brief Time-of-flight cut using momentum-dependent parameterization
     *
     * This uses a 3-sigma cut around a momentum-dependent mean with
     * parameterized sigma values from pion calibration studies.
     */
    auto pass_delta_t_cut(double p, double delta_t) -> bool {
        // Parameterization from pion timing studies
        const double popt_mean[4] = {-0.02872196, 0.00598219, 0.00158193, -0.00028493};
        const double popt_sigma[4] = {0.25132972, -0.09788015, 0.01734634, -0.00103597};

        double mean = third_order_poly(p, popt_mean[0], popt_mean[1], popt_mean[2], popt_mean[3]);
        double sigma = third_order_poly(p, popt_sigma[0], popt_sigma[1], popt_sigma[2], popt_sigma[3]);

        double cut_low = mean - 3.0 * sigma;
        double cut_high = mean + 3.0 * sigma;

        return cut_low < delta_t && delta_t < cut_high;
    }

    auto fill_histograms(const CLAS::Particle& pion, double delta_t, bool is_piplus) -> void {
        double p = pion.p();
        double theta = pion.theta();
        double phi = pion.phi();
        double vz = pion.vz();
        double chi2pid = pion.chi2pid();
        double beta = pion.beta();

        if (is_piplus) {
            // Fill π⁺ histograms
            h_piplus.momentum->Fill(p);
            h_piplus.theta->Fill(theta);
            h_piplus.phi->Fill(phi);
            h_piplus.vz->Fill(vz);
            h_piplus.chi2pid->Fill(chi2pid);
            h_piplus.beta->Fill(beta);
            h_piplus.beta_vs_p->Fill(p, beta);
            h_piplus.delta_t_vs_p->Fill(p, delta_t);
            h_piplus.p_vs_theta->Fill(theta, p);

            // Fill π⁺ N-tuple
            if (m_ntuple_svc) {
                m_nt_piplus_p = p;
                m_nt_piplus_theta = theta;
                m_nt_piplus_phi = phi;
                m_nt_piplus_vz = vz;
                m_nt_piplus_chi2pid = chi2pid;
                m_nt_piplus_beta = beta;
                m_nt_piplus_delta_t = delta_t;
                m_ntuple_svc->fill("pion_plus");
            }
        } else {
            // Fill π⁻ histograms
            h_piminus.momentum->Fill(p);
            h_piminus.theta->Fill(theta);
            h_piminus.phi->Fill(phi);
            h_piminus.vz->Fill(vz);
            h_piminus.chi2pid->Fill(chi2pid);
            h_piminus.beta->Fill(beta);
            h_piminus.beta_vs_p->Fill(p, beta);
            h_piminus.delta_t_vs_p->Fill(p, delta_t);
            h_piminus.p_vs_theta->Fill(theta, p);

            // Fill π⁻ N-tuple
            if (m_ntuple_svc) {
                m_nt_piminus_p = p;
                m_nt_piminus_theta = theta;
                m_nt_piminus_phi = phi;
                m_nt_piminus_vz = vz;
                m_nt_piminus_chi2pid = chi2pid;
                m_nt_piminus_beta = beta;
                m_nt_piminus_delta_t = delta_t;
                m_ntuple_svc->fill("pion_minus");
            }
        }
    }
};

}  // namespace Algorithms
