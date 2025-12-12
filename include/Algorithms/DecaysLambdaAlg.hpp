#pragma once

#include <cmath>
#include <mutex>
#include <optional>
#include <vector>
#include <Math/Vector4D.h>
#include "CLAS/Constants.hpp"
#include "CLAS/PIDs.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"

namespace Algorithms {

using Framework::Log;

/**
 * @brief Lambda PDG code
 */
constexpr int LAMBDA_PID = 3122;
constexpr double LAMBDA_MASS_PDG = 1.115683;  // GeV/c²

/**
 * @brief Reconstructed Lambda from DECAYS::Particle bank
 * 
 * The DECAYS::Particle bank contains pre-reconstructed decay particles with:
 * - pid: particle ID (3122 for Lambda)
 * - mass/umass: reconstructed mass
 * - px, py, pz: momentum components
 * - vx, vy, vz: decay vertex
 * - ovx, ovy, ovz: origin vertex (production point)
 * - ndau, dau1idx, dau2idx: daughter indices
 * - e, emc, erec: energy values
 */
struct DecaysLambda {
    int idx = -1;                 // Index in DECAYS::Particle bank
    int pid = 0;                  // Should be 3122 for Lambda
    double mass = 0.0;            // Reconstructed mass
    double umass = 0.0;           // Unmeasured mass
    double px = 0.0, py = 0.0, pz = 0.0;  // Momentum
    double vx = 0.0, vy = 0.0, vz = 0.0;  // Decay vertex
    double ovx = 0.0, ovy = 0.0, ovz = 0.0;  // Origin vertex
    double e = 0.0;               // Energy
    double emc = 0.0;             // MC energy
    double erec = 0.0;            // Reconstructed energy
    int charge = 0;
    int ndau = 0;                 // Number of daughters
    int dau1idx = 0, dau2idx = 0, dau3idx = 0;  // Daughter indices
    int det = 0;                  // Detector info
    double r = 0.0;               // Decay radius
    
    // Derived quantities
    double momentum() const { return std::sqrt(px*px + py*py + pz*pz); }
    double theta() const { return std::acos(pz / momentum()); }
    double phi() const { return std::atan2(py, px); }
    double decay_length() const {
        double dx = vx - ovx;
        double dy = vy - ovy;
        double dz = vz - ovz;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
    
    // Vertex detachment: z displacement from origin to decay vertex
    double vertex_detach_z() const { return vz - ovz; }
    
    // Transverse vertex detachment
    double vertex_detach_xy() const {
        double dx = vx - ovx;
        double dy = vy - ovy;
        return std::sqrt(dx*dx + dy*dy);
    }
};

/**
 * @brief Daughter particle from DECAYS::Particle bank
 */
struct DecaysDaughter {
    int idx = -1;
    int pid = 0;
    double mass = 0.0;
    double px = 0.0, py = 0.0, pz = 0.0;
    double vx = 0.0, vy = 0.0, vz = 0.0;
    double upx = 0.0, upy = 0.0, upz = 0.0;  // Unmeasured momentum
    int charge = 0;
    int det = 0;
    
    double momentum() const { return std::sqrt(px*px + py*py + pz*pz); }
    double theta() const { return std::acos(pz / momentum()); }
    double phi() const { return std::atan2(py, px); }
};

/**
 * @brief Algorithm for Lambda analysis using DECAYS::Particle bank
 *
 * This algorithm reads pre-reconstructed Lambda candidates from the
 * DECAYS::Particle bank and performs analysis including:
 * - Mass distribution studies
 * - Kinematic distributions
 * - Decay vertex analysis
 * - Daughter particle correlations
 */
class DecaysLambdaAlg : public Framework::Algorithm {
public:
    explicit DecaysLambdaAlg(std::string name = "DecaysLambdaAlg")
        : Algorithm(std::move(name)), m_cutflow("DECAYS Lambda Analysis") {}

    void set_mass_window(double window) { m_mass_window = window; }
    void set_min_momentum(double p) { m_min_momentum = p; }
    void set_min_decay_length(double l) { m_min_decay_length = l; }
    void set_min_vertex_detach(double d) { m_min_vertex_detach = d; }
    void set_max_vertex_detach(double d) { m_max_vertex_detach = d; }

    auto initialize() -> Framework::StatusCode override {
        auto log_sc = initialize_handle(m_log_svc);
        if (log_sc.is_failure()) Log::error(name(), "WARNING: LoggingService not available");

        Log::info(name(), "Initializing DECAYS Lambda analysis algorithm");
        Log::info(name(), "  Mass window: {} GeV", m_mass_window);
        Log::info(name(), "  Min momentum: {} GeV", m_min_momentum);
        Log::info(name(), "  Min decay length: {} cm", m_min_decay_length);
        Log::info(name(), "  Min vertex detach: {} cm", m_min_vertex_detach);
        Log::info(name(), "  Max vertex detach: {} cm", m_max_vertex_detach);

        auto hist_sc = initialize_handle(m_hist_svc);
        if (hist_sc.is_failure()) {
            Log::error(name(), "Could not get HistogramService: {}", hist_sc.message());
            return hist_sc;
        }

        auto ntuple_sc = initialize_handle(m_ntuple_svc);
        if (ntuple_sc.is_failure()) {
            Log::warning(name(), "Could not get NTupleService - N-tuple output disabled");
        }

        book_histograms();
        if (m_ntuple_svc) setup_ntuple();

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has DECAYS Bank");
        m_cutflow.add_cut("Has Lambda");
        m_cutflow.add_cut("Mass Window");
        m_cutflow.add_cut("Momentum Cut");
        m_cutflow.add_cut("Decay Length Cut");
        m_cutflow.add_cut("Vertex Detach Cut");

        m_cutflow.set_logger(m_log_svc.get());
        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get DECAYS::Particle bank
        auto& hipo_event = event.hipo_event();
        auto& decays_bank = hipo_event["DECAYS::Particle"];
        
        const int nrows = decays_bank.rows();
        if (nrows == 0) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Has DECAYS Bank");

        // Read all particles from DECAYS bank and find Lambdas
        std::vector<DecaysLambda> lambdas;
        std::vector<DecaysDaughter> all_daughters;
        
        for (int i = 0; i < nrows; ++i) {
            int pid = decays_bank.get_as<int>("pid", i);
            
            if (pid == LAMBDA_PID) {
                DecaysLambda lambda;
                lambda.idx = decays_bank.get_as<int>("idx", i);
                lambda.pid = pid;
                lambda.mass = decays_bank.get_as<float>("mass", i);
                lambda.umass = decays_bank.get_as<float>("umass", i);
                lambda.px = decays_bank.get_as<float>("px", i);
                lambda.py = decays_bank.get_as<float>("py", i);
                lambda.pz = decays_bank.get_as<float>("pz", i);
                lambda.vx = decays_bank.get_as<float>("vx", i);
                lambda.vy = decays_bank.get_as<float>("vy", i);
                lambda.vz = decays_bank.get_as<float>("vz", i);
                lambda.ovx = decays_bank.get_as<float>("ovx", i);
                lambda.ovy = decays_bank.get_as<float>("ovy", i);
                lambda.ovz = decays_bank.get_as<float>("ovz", i);
                lambda.e = decays_bank.get_as<float>("e", i);
                lambda.emc = decays_bank.get_as<float>("emc", i);
                lambda.erec = decays_bank.get_as<float>("erec", i);
                lambda.charge = decays_bank.get_as<int>("charge", i);
                lambda.ndau = decays_bank.get_as<int>("ndau", i);
                lambda.dau1idx = decays_bank.get_as<int>("dau1idx", i);
                lambda.dau2idx = decays_bank.get_as<int>("dau2idx", i);
                lambda.dau3idx = decays_bank.get_as<int>("dau3idx", i);
                lambda.det = decays_bank.get_as<int>("det", i);
                lambda.r = decays_bank.get_as<float>("r", i);
                
                lambdas.push_back(lambda);
            } else {
                // Store as potential daughter
                DecaysDaughter dau;
                dau.idx = decays_bank.get_as<int>("idx", i);
                dau.pid = pid;
                dau.mass = decays_bank.get_as<float>("mass", i);
                dau.px = decays_bank.get_as<float>("px", i);
                dau.py = decays_bank.get_as<float>("py", i);
                dau.pz = decays_bank.get_as<float>("pz", i);
                dau.vx = decays_bank.get_as<float>("vx", i);
                dau.vy = decays_bank.get_as<float>("vy", i);
                dau.vz = decays_bank.get_as<float>("vz", i);
                dau.upx = decays_bank.get_as<float>("upx", i);
                dau.upy = decays_bank.get_as<float>("upy", i);
                dau.upz = decays_bank.get_as<float>("upz", i);
                dau.charge = decays_bank.get_as<int>("charge", i);
                dau.det = decays_bank.get_as<int>("det", i);
                
                all_daughters.push_back(dau);
            }
        }

        if (lambdas.empty()) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Has Lambda");

        // Fill pre-cut histograms
        if (h_n_lambdas) h_n_lambdas->Fill(lambdas.size());
        for (const auto& lambda : lambdas) {
            if (h_mass_nocuts) h_mass_nocuts->Fill(lambda.mass);
        }

        // Apply selection cuts
        std::vector<DecaysLambda> selected_lambdas;
        bool passed_mass = false;
        bool passed_momentum = false;
        bool passed_decay_length = false;
        bool passed_vertex_detach = false;

        for (const auto& lambda : lambdas) {
            // Fill pre-cut vertex detachment histogram
            if (h_vertex_detach_precut) h_vertex_detach_precut->Fill(lambda.vertex_detach_z());
            
            // Mass window cut
            if (std::abs(lambda.mass - LAMBDA_MASS_PDG) > m_mass_window) continue;
            passed_mass = true;
            
            // Momentum cut
            if (lambda.momentum() < m_min_momentum) continue;
            passed_momentum = true;
            
            // Decay length cut (if enabled)
            if (m_min_decay_length > 0 && lambda.decay_length() < m_min_decay_length) continue;
            passed_decay_length = true;
            
            // Vertex detachment cut (if enabled)
            // Requires decay vertex to be displaced from origin vertex
            if (m_min_vertex_detach > -999) {
                double detach = lambda.vertex_detach_z();
                if (detach < m_min_vertex_detach) continue;
                if (m_max_vertex_detach > 0 && detach > m_max_vertex_detach) continue;
            }
            passed_vertex_detach = true;
            
            selected_lambdas.push_back(lambda);
        }

        if (!passed_mass) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Mass Window");

        if (!passed_momentum) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Momentum Cut");

        if (m_min_decay_length > 0 && !passed_decay_length) return Framework::StatusCode::SUCCESS();
        if (m_min_decay_length > 0) m_cutflow.pass("Decay Length Cut");
        
        if (m_min_vertex_detach > -999 && !passed_vertex_detach) return Framework::StatusCode::SUCCESS();
        if (m_min_vertex_detach > -999) m_cutflow.pass("Vertex Detach Cut");

        if (selected_lambdas.empty()) return Framework::StatusCode::SUCCESS();

        // Store results
        event.set_decision("HasDecaysLambda", true);
        
        // Find best candidate (closest to PDG mass)
        auto best_it = std::min_element(selected_lambdas.begin(), selected_lambdas.end(),
            [](const DecaysLambda& a, const DecaysLambda& b) {
                return std::abs(a.mass - LAMBDA_MASS_PDG) < std::abs(b.mass - LAMBDA_MASS_PDG);
            });

        event.store("decays_lambda_mass", best_it->mass);
        event.store("decays_lambda_p", best_it->momentum());

        // Fill histograms for selected Lambdas
        for (const auto& lambda : selected_lambdas) {
            fill_lambda_histograms(lambda, all_daughters);
        }

        // Fill N-tuple
        if (m_ntuple_svc) {
            fill_ntuple(selected_lambdas, all_daughters);
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        Log::info(name(), "Finalizing DECAYS Lambda analysis algorithm");
        if (m_ntuple_svc) {
            auto entries = m_ntuple_svc->get_entries("decays_lambda");
            Log::info(name(), "N-tuple has {} entries", entries);
        }
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

private:
    double m_mass_window = 0.1;       // GeV (±100 MeV around PDG mass)
    double m_min_momentum = 0.0;      // GeV
    double m_min_decay_length = 0.0;  // cm (0 = disabled)
    double m_min_vertex_detach = -1000.0;  // cm - min z displacement (decay_vz - origin_vz), -1000 = disabled
    double m_max_vertex_detach = 0.0;      // cm - max z displacement, 0 = no upper limit
    Framework::Cutflow m_cutflow;

    // Services
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};

    mutable std::mutex m_ntuple_mutex;

    // Histograms
    TH1D* h_n_lambdas = nullptr;
    TH1D* h_mass_nocuts = nullptr;
    TH1D* h_mass_selected = nullptr;
    TH1D* h_umass = nullptr;
    TH1D* h_momentum = nullptr;
    TH1D* h_theta = nullptr;
    TH1D* h_phi = nullptr;
    TH1D* h_decay_length = nullptr;
    TH1D* h_decay_vz = nullptr;
    TH1D* h_origin_vz = nullptr;
    TH1D* h_r = nullptr;
    TH1D* h_ndau = nullptr;
    TH1D* h_proton_p = nullptr;
    TH1D* h_pion_p = nullptr;
    TH2D* h_mass_vs_p = nullptr;
    TH2D* h_decay_vz_vs_origin_vz = nullptr;
    TH1D* h_vertex_detach = nullptr;
    TH1D* h_vertex_detach_precut = nullptr;
    TH1D* h_vertex_detach_xy = nullptr;

    // N-tuple variables
    std::vector<double> m_nt_mass;
    std::vector<double> m_nt_umass;
    std::vector<double> m_nt_p;
    std::vector<double> m_nt_theta;
    std::vector<double> m_nt_phi;
    std::vector<double> m_nt_decay_length;
    std::vector<double> m_nt_vz;
    std::vector<double> m_nt_ovz;
    std::vector<double> m_nt_proton_p;
    std::vector<double> m_nt_pion_p;
    std::vector<double> m_nt_vertex_detach;

    void book_histograms() {
        h_n_lambdas = m_hist_svc->book_hist1D("/DecaysLambda/n_lambdas", 
            "Number of #Lambda per event;N_{#Lambda};Events", 10, 0, 10);
        h_mass_nocuts = m_hist_svc->book_hist1D("/DecaysLambda/mass_nocuts", 
            "#Lambda mass (no cuts);M [GeV];Events", 200, 1.0, 1.3);
        h_mass_selected = m_hist_svc->book_hist1D("/DecaysLambda/mass_selected", 
            "#Lambda mass (selected);M [GeV];Events", 200, 1.0, 1.3);
        h_umass = m_hist_svc->book_hist1D("/DecaysLambda/umass", 
            "#Lambda unmeasured mass;M [GeV];Events", 200, 1.0, 1.3);
        h_momentum = m_hist_svc->book_hist1D("/DecaysLambda/momentum", 
            "#Lambda momentum;p [GeV];Events", 100, 0, 10);
        h_theta = m_hist_svc->book_hist1D("/DecaysLambda/theta", 
            "#Lambda #theta;#theta [deg];Events", 90, 0, 90);
        h_phi = m_hist_svc->book_hist1D("/DecaysLambda/phi", 
            "#Lambda #phi;#phi [deg];Events", 180, -180, 180);
        h_decay_length = m_hist_svc->book_hist1D("/DecaysLambda/decay_length", 
            "#Lambda decay length;L [cm];Events", 100, 0, 50);
        h_decay_vz = m_hist_svc->book_hist1D("/DecaysLambda/decay_vz", 
            "#Lambda decay vertex z;v_{z} [cm];Events", 200, -30, 30);
        h_origin_vz = m_hist_svc->book_hist1D("/DecaysLambda/origin_vz", 
            "#Lambda origin vertex z;ov_{z} [cm];Events", 200, -30, 30);
        h_r = m_hist_svc->book_hist1D("/DecaysLambda/r", 
            "#Lambda decay radius;r [cm];Events", 100, 0, 20);
        h_ndau = m_hist_svc->book_hist1D("/DecaysLambda/ndau", 
            "Number of daughters;N_{dau};Events", 5, 0, 5);
        h_proton_p = m_hist_svc->book_hist1D("/DecaysLambda/proton_p", 
            "Proton momentum;p [GeV];Events", 100, 0, 5);
        h_pion_p = m_hist_svc->book_hist1D("/DecaysLambda/pion_p", 
            "#pi^{-} momentum;p [GeV];Events", 100, 0, 3);
        h_mass_vs_p = m_hist_svc->book_hist2D("/DecaysLambda/mass_vs_p", 
            "#Lambda mass vs momentum;p [GeV];M [GeV]", 100, 0, 10, 100, 1.0, 1.3);
        h_decay_vz_vs_origin_vz = m_hist_svc->book_hist2D("/DecaysLambda/decay_vz_vs_origin_vz", 
            "Decay vs origin vertex z;ov_{z} [cm];v_{z} [cm]", 100, -30, 30, 100, -30, 30);
        h_vertex_detach = m_hist_svc->book_hist1D("/DecaysLambda/vertex_detach", 
            "Vertex detachment (v_{z} - ov_{z});#Deltaz [cm];Events", 200, -20, 40);
        h_vertex_detach_precut = m_hist_svc->book_hist1D("/DecaysLambda/vertex_detach_precut", 
            "Vertex detachment (pre-cut);#Deltaz [cm];Events", 200, -20, 40);
        h_vertex_detach_xy = m_hist_svc->book_hist1D("/DecaysLambda/vertex_detach_xy", 
            "Transverse vertex detachment;#Deltar_{xy} [cm];Events", 100, 0, 20);
    }

    void fill_lambda_histograms(const DecaysLambda& lambda, const std::vector<DecaysDaughter>& daughters) {
        if (h_mass_selected) h_mass_selected->Fill(lambda.mass);
        if (h_umass) h_umass->Fill(lambda.umass);
        if (h_momentum) h_momentum->Fill(lambda.momentum());
        if (h_theta) h_theta->Fill(lambda.theta() * CLAS::Constants::RAD_TO_DEG);
        if (h_phi) h_phi->Fill(lambda.phi() * CLAS::Constants::RAD_TO_DEG);
        if (h_decay_length) h_decay_length->Fill(lambda.decay_length());
        if (h_decay_vz) h_decay_vz->Fill(lambda.vz);
        if (h_origin_vz) h_origin_vz->Fill(lambda.ovz);
        if (h_r) h_r->Fill(lambda.r);
        if (h_ndau) h_ndau->Fill(lambda.ndau);
        if (h_mass_vs_p) h_mass_vs_p->Fill(lambda.momentum(), lambda.mass);
        if (h_decay_vz_vs_origin_vz) h_decay_vz_vs_origin_vz->Fill(lambda.ovz, lambda.vz);
        if (h_vertex_detach) h_vertex_detach->Fill(lambda.vertex_detach_z());
        if (h_vertex_detach_xy) h_vertex_detach_xy->Fill(lambda.vertex_detach_xy());

        // Find and fill daughter histograms
        for (const auto& dau : daughters) {
            if (dau.idx == lambda.dau1idx || dau.idx == lambda.dau2idx) {
                if (dau.pid == static_cast<int>(CLAS::PIDs::PROTON)) {
                    if (h_proton_p) h_proton_p->Fill(dau.momentum());
                } else if (dau.pid == static_cast<int>(CLAS::PIDs::PIMINUS)) {
                    if (h_pion_p) h_pion_p->Fill(dau.momentum());
                }
            }
        }
    }

    void setup_ntuple() {
        m_ntuple_svc->register_branch("decays_lambda", "mass", &m_nt_mass);
        m_ntuple_svc->register_branch("decays_lambda", "umass", &m_nt_umass);
        m_ntuple_svc->register_branch("decays_lambda", "p", &m_nt_p);
        m_ntuple_svc->register_branch("decays_lambda", "theta", &m_nt_theta);
        m_ntuple_svc->register_branch("decays_lambda", "phi", &m_nt_phi);
        m_ntuple_svc->register_branch("decays_lambda", "decay_length", &m_nt_decay_length);
        m_ntuple_svc->register_branch("decays_lambda", "vz", &m_nt_vz);
        m_ntuple_svc->register_branch("decays_lambda", "ovz", &m_nt_ovz);
        m_ntuple_svc->register_branch("decays_lambda", "proton_p", &m_nt_proton_p);
        m_ntuple_svc->register_branch("decays_lambda", "pion_p", &m_nt_pion_p);
        m_ntuple_svc->register_branch("decays_lambda", "vertex_detach", &m_nt_vertex_detach);
        Log::info(name(), "N-tuple branches registered for DECAYS Lambda analysis");
    }

    void fill_ntuple(const std::vector<DecaysLambda>& lambdas, const std::vector<DecaysDaughter>& daughters) {
        std::lock_guard<std::mutex> lock(m_ntuple_mutex);
        
        m_nt_mass.clear();
        m_nt_umass.clear();
        m_nt_p.clear();
        m_nt_theta.clear();
        m_nt_phi.clear();
        m_nt_decay_length.clear();
        m_nt_vz.clear();
        m_nt_ovz.clear();
        m_nt_proton_p.clear();
        m_nt_pion_p.clear();
        m_nt_vertex_detach.clear();

        for (const auto& lambda : lambdas) {
            m_nt_mass.push_back(lambda.mass);
            m_nt_umass.push_back(lambda.umass);
            m_nt_p.push_back(lambda.momentum());
            m_nt_theta.push_back(lambda.theta());
            m_nt_phi.push_back(lambda.phi());
            m_nt_decay_length.push_back(lambda.decay_length());
            m_nt_vz.push_back(lambda.vz);
            m_nt_ovz.push_back(lambda.ovz);
            m_nt_vertex_detach.push_back(lambda.vertex_detach_z());

            // Find daughters
            double proton_p = 0.0, pion_p = 0.0;
            for (const auto& dau : daughters) {
                if (dau.idx == lambda.dau1idx || dau.idx == lambda.dau2idx) {
                    if (dau.pid == static_cast<int>(CLAS::PIDs::PROTON)) {
                        proton_p = dau.momentum();
                    } else if (dau.pid == static_cast<int>(CLAS::PIDs::PIMINUS)) {
                        pion_p = dau.momentum();
                    }
                }
            }
            m_nt_proton_p.push_back(proton_p);
            m_nt_pion_p.push_back(pion_p);
        }

        m_ntuple_svc->fill("decays_lambda");
    }
};

}  // namespace Algorithms
