#pragma once

/**
 * @file AiPidHadronAlg.hpp
 * @brief Algorithm for AI PID hadron analysis using RICH detector
 *
 * Reads the RICH::Particle bank to identify pions and kaons
 * based on the RICH best_PID and log-likelihood information.
 * Outputs a ROOT TTree with particle, calorimeter, cherenkov,
 * and scintillator data, using RICH best_PID as the truth PID.
 */

#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>
#include <clas12/Particle.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/ReadBank.hpp>

#include <cmath>
#include <string>
#include <fmt/core.h>

namespace ai_pid_hadron {

/**
 * @brief N-Tuple data structure for RICH hadron analysis
 *
 * Mirrors the TreeData fields from the AI PID reader,
 * with rich_pid replacing mc_pid.
 */
struct HadronTreeData {
    // Particle identification
    int rich_pid = 0;    // RICH best_PID (replaces mc_pid)
    int pid = 0;         // REC::Particle PID
    float p = 0;
    float theta = 0;
    float phi = 0;
    float beta = 0;
    int charge = 0;

    // Calorimeter - PCAL
    float PCAL_E = 0;
    float PCAL_chi2 = 0;
    float PCAL_time = 0;
    float PCAL_path = 0;
    float PCAL_lu = 0;
    float PCAL_lv = 0;
    float PCAL_lw = 0;
    float PCAL_du = 0;
    float PCAL_dv = 0;
    float PCAL_dw = 0;
    float PCAL_m2u = 0;
    float PCAL_m2v = 0;
    float PCAL_m2w = 0;
    float PCAL_m3u = 0;
    float PCAL_m3v = 0;
    float PCAL_m3w = 0;
    short PCAL_dbstU = 0;
    short PCAL_dbstV = 0;
    short PCAL_dbstW = 0;
    float PCAL_rawEU = 0;
    float PCAL_rawEV = 0;
    float PCAL_rawEW = 0;
    float PCAL_recEU = 0;
    float PCAL_recEV = 0;
    float PCAL_recEW = 0;
    float PCAL_recDTU = 0;
    float PCAL_recDTV = 0;
    float PCAL_recDTW = 0;
    float PCAL_recFTU = 0;
    float PCAL_recFTV = 0;
    float PCAL_recFTW = 0;
    float PCAL_size = 0;

    // Calorimeter - Inner (ECIN)
    float ICAL_E = 0;
    float ICAL_chi2 = 0;
    float ICAL_time = 0;
    float ICAL_path = 0;
    float ICAL_lu = 0;
    float ICAL_lv = 0;
    float ICAL_lw = 0;
    float ICAL_du = 0;
    float ICAL_dv = 0;
    float ICAL_dw = 0;
    float ICAL_m2u = 0;
    float ICAL_m2v = 0;
    float ICAL_m2w = 0;
    float ICAL_m3u = 0;
    float ICAL_m3v = 0;
    float ICAL_m3w = 0;
    short ICAL_dbstU = 0;
    short ICAL_dbstV = 0;
    short ICAL_dbstW = 0;
    float ICAL_rawEU = 0;
    float ICAL_rawEV = 0;
    float ICAL_rawEW = 0;
    float ICAL_recEU = 0;
    float ICAL_recEV = 0;
    float ICAL_recEW = 0;
    float ICAL_recDTU = 0;
    float ICAL_recDTV = 0;
    float ICAL_recDTW = 0;
    float ICAL_recFTU = 0;
    float ICAL_recFTV = 0;
    float ICAL_recFTW = 0;
    float ICAL_size = 0;

    // Calorimeter - Outer (ECOUT)
    float OCAL_E = 0;
    float OCAL_chi2 = 0;
    float OCAL_time = 0;
    float OCAL_path = 0;
    float OCAL_lu = 0;
    float OCAL_lv = 0;
    float OCAL_lw = 0;
    float OCAL_du = 0;
    float OCAL_dv = 0;
    float OCAL_dw = 0;
    float OCAL_m2u = 0;
    float OCAL_m2v = 0;
    float OCAL_m2w = 0;
    float OCAL_m3u = 0;
    float OCAL_m3v = 0;
    float OCAL_m3w = 0;
    short OCAL_dbstU = 0;
    short OCAL_dbstV = 0;
    short OCAL_dbstW = 0;
    float OCAL_rawEU = 0;
    float OCAL_rawEV = 0;
    float OCAL_rawEW = 0;
    float OCAL_recEU = 0;
    float OCAL_recEV = 0;
    float OCAL_recEW = 0;
    float OCAL_recDTU = 0;
    float OCAL_recDTV = 0;
    float OCAL_recDTW = 0;
    float OCAL_recFTU = 0;
    float OCAL_recFTV = 0;
    float OCAL_recFTW = 0;
    float OCAL_size = 0;

    // Cherenkov (HTCC)
    float HTCC_nphe = 0;
    float HTCC_time = 0;
    float HTCC_path = 0;
    float HTCC_chi2 = 0;
    float HTCC_dtheta = 0;
    float HTCC_dphi = 0;

    // Scintillator (FTOF)
    float FTOF_E = 0;
    float FTOF_time = 0;
    float FTOF_path = 0;
    float FTOF_chi2 = 0;
    float FTOF_dedx = 0;
    short FTOF_size = 0;
    char FTOF_layermult = 0;

    template <typename Svc>
    auto register_branches(Svc* svc, const std::string& tree) -> void {
        // Particle
        svc->register_branch(tree, "rich_pid", &rich_pid);
        svc->register_branch(tree, "pid", &pid);
        svc->register_branch(tree, "p", &p);
        svc->register_branch(tree, "theta", &theta);
        svc->register_branch(tree, "phi", &phi);
        svc->register_branch(tree, "beta", &beta);
        svc->register_branch(tree, "charge", &charge);

        // PCAL
        svc->register_branch(tree, "PCAL_E", &PCAL_E);
        svc->register_branch(tree, "PCAL_chi2", &PCAL_chi2);
        svc->register_branch(tree, "PCAL_time", &PCAL_time);
        svc->register_branch(tree, "PCAL_path", &PCAL_path);
        svc->register_branch(tree, "PCAL_lu", &PCAL_lu);
        svc->register_branch(tree, "PCAL_lv", &PCAL_lv);
        svc->register_branch(tree, "PCAL_lw", &PCAL_lw);
        svc->register_branch(tree, "PCAL_du", &PCAL_du);
        svc->register_branch(tree, "PCAL_dv", &PCAL_dv);
        svc->register_branch(tree, "PCAL_dw", &PCAL_dw);
        svc->register_branch(tree, "PCAL_m2u", &PCAL_m2u);
        svc->register_branch(tree, "PCAL_m2v", &PCAL_m2v);
        svc->register_branch(tree, "PCAL_m2w", &PCAL_m2w);
        svc->register_branch(tree, "PCAL_m3u", &PCAL_m3u);
        svc->register_branch(tree, "PCAL_m3v", &PCAL_m3v);
        svc->register_branch(tree, "PCAL_m3w", &PCAL_m3w);
        svc->register_branch(tree, "PCAL_dbstU", &PCAL_dbstU);
        svc->register_branch(tree, "PCAL_dbstV", &PCAL_dbstV);
        svc->register_branch(tree, "PCAL_dbstW", &PCAL_dbstW);
        svc->register_branch(tree, "PCAL_rawEU", &PCAL_rawEU);
        svc->register_branch(tree, "PCAL_rawEV", &PCAL_rawEV);
        svc->register_branch(tree, "PCAL_rawEW", &PCAL_rawEW);
        svc->register_branch(tree, "PCAL_recEU", &PCAL_recEU);
        svc->register_branch(tree, "PCAL_recEV", &PCAL_recEV);
        svc->register_branch(tree, "PCAL_recEW", &PCAL_recEW);
        svc->register_branch(tree, "PCAL_recDTU", &PCAL_recDTU);
        svc->register_branch(tree, "PCAL_recDTV", &PCAL_recDTV);
        svc->register_branch(tree, "PCAL_recDTW", &PCAL_recDTW);
        svc->register_branch(tree, "PCAL_recFTU", &PCAL_recFTU);
        svc->register_branch(tree, "PCAL_recFTV", &PCAL_recFTV);
        svc->register_branch(tree, "PCAL_recFTW", &PCAL_recFTW);
        svc->register_branch(tree, "PCAL_size", &PCAL_size);

        // ECIN
        svc->register_branch(tree, "ICAL_E", &ICAL_E);
        svc->register_branch(tree, "ICAL_chi2", &ICAL_chi2);
        svc->register_branch(tree, "ICAL_time", &ICAL_time);
        svc->register_branch(tree, "ICAL_path", &ICAL_path);
        svc->register_branch(tree, "ICAL_lu", &ICAL_lu);
        svc->register_branch(tree, "ICAL_lv", &ICAL_lv);
        svc->register_branch(tree, "ICAL_lw", &ICAL_lw);
        svc->register_branch(tree, "ICAL_du", &ICAL_du);
        svc->register_branch(tree, "ICAL_dv", &ICAL_dv);
        svc->register_branch(tree, "ICAL_dw", &ICAL_dw);
        svc->register_branch(tree, "ICAL_m2u", &ICAL_m2u);
        svc->register_branch(tree, "ICAL_m2v", &ICAL_m2v);
        svc->register_branch(tree, "ICAL_m2w", &ICAL_m2w);
        svc->register_branch(tree, "ICAL_m3u", &ICAL_m3u);
        svc->register_branch(tree, "ICAL_m3v", &ICAL_m3v);
        svc->register_branch(tree, "ICAL_m3w", &ICAL_m3w);
        svc->register_branch(tree, "ICAL_dbstU", &ICAL_dbstU);
        svc->register_branch(tree, "ICAL_dbstV", &ICAL_dbstV);
        svc->register_branch(tree, "ICAL_dbstW", &ICAL_dbstW);
        svc->register_branch(tree, "ICAL_rawEU", &ICAL_rawEU);
        svc->register_branch(tree, "ICAL_rawEV", &ICAL_rawEV);
        svc->register_branch(tree, "ICAL_rawEW", &ICAL_rawEW);
        svc->register_branch(tree, "ICAL_recEU", &ICAL_recEU);
        svc->register_branch(tree, "ICAL_recEV", &ICAL_recEV);
        svc->register_branch(tree, "ICAL_recEW", &ICAL_recEW);
        svc->register_branch(tree, "ICAL_recDTU", &ICAL_recDTU);
        svc->register_branch(tree, "ICAL_recDTV", &ICAL_recDTV);
        svc->register_branch(tree, "ICAL_recDTW", &ICAL_recDTW);
        svc->register_branch(tree, "ICAL_recFTU", &ICAL_recFTU);
        svc->register_branch(tree, "ICAL_recFTV", &ICAL_recFTV);
        svc->register_branch(tree, "ICAL_recFTW", &ICAL_recFTW);
        svc->register_branch(tree, "ICAL_size", &ICAL_size);

        // ECOUT
        svc->register_branch(tree, "OCAL_E", &OCAL_E);
        svc->register_branch(tree, "OCAL_chi2", &OCAL_chi2);
        svc->register_branch(tree, "OCAL_time", &OCAL_time);
        svc->register_branch(tree, "OCAL_path", &OCAL_path);
        svc->register_branch(tree, "OCAL_lu", &OCAL_lu);
        svc->register_branch(tree, "OCAL_lv", &OCAL_lv);
        svc->register_branch(tree, "OCAL_lw", &OCAL_lw);
        svc->register_branch(tree, "OCAL_du", &OCAL_du);
        svc->register_branch(tree, "OCAL_dv", &OCAL_dv);
        svc->register_branch(tree, "OCAL_dw", &OCAL_dw);
        svc->register_branch(tree, "OCAL_m2u", &OCAL_m2u);
        svc->register_branch(tree, "OCAL_m2v", &OCAL_m2v);
        svc->register_branch(tree, "OCAL_m2w", &OCAL_m2w);
        svc->register_branch(tree, "OCAL_m3u", &OCAL_m3u);
        svc->register_branch(tree, "OCAL_m3v", &OCAL_m3v);
        svc->register_branch(tree, "OCAL_m3w", &OCAL_m3w);
        svc->register_branch(tree, "OCAL_dbstU", &OCAL_dbstU);
        svc->register_branch(tree, "OCAL_dbstV", &OCAL_dbstV);
        svc->register_branch(tree, "OCAL_dbstW", &OCAL_dbstW);
        svc->register_branch(tree, "OCAL_rawEU", &OCAL_rawEU);
        svc->register_branch(tree, "OCAL_rawEV", &OCAL_rawEV);
        svc->register_branch(tree, "OCAL_rawEW", &OCAL_rawEW);
        svc->register_branch(tree, "OCAL_recEU", &OCAL_recEU);
        svc->register_branch(tree, "OCAL_recEV", &OCAL_recEV);
        svc->register_branch(tree, "OCAL_recEW", &OCAL_recEW);
        svc->register_branch(tree, "OCAL_recDTU", &OCAL_recDTU);
        svc->register_branch(tree, "OCAL_recDTV", &OCAL_recDTV);
        svc->register_branch(tree, "OCAL_recDTW", &OCAL_recDTW);
        svc->register_branch(tree, "OCAL_recFTU", &OCAL_recFTU);
        svc->register_branch(tree, "OCAL_recFTV", &OCAL_recFTV);
        svc->register_branch(tree, "OCAL_recFTW", &OCAL_recFTW);
        svc->register_branch(tree, "OCAL_size", &OCAL_size);

        // HTCC
        svc->register_branch(tree, "HTCC_nphe", &HTCC_nphe);
        svc->register_branch(tree, "HTCC_time", &HTCC_time);
        svc->register_branch(tree, "HTCC_path", &HTCC_path);
        svc->register_branch(tree, "HTCC_chi2", &HTCC_chi2);
        svc->register_branch(tree, "HTCC_dtheta", &HTCC_dtheta);
        svc->register_branch(tree, "HTCC_dphi", &HTCC_dphi);

        // FTOF
        svc->register_branch(tree, "FTOF_E", &FTOF_E);
        svc->register_branch(tree, "FTOF_time", &FTOF_time);
        svc->register_branch(tree, "FTOF_path", &FTOF_path);
        svc->register_branch(tree, "FTOF_chi2", &FTOF_chi2);
        svc->register_branch(tree, "FTOF_dedx", &FTOF_dedx);
        svc->register_branch(tree, "FTOF_size", &FTOF_size);
        svc->register_branch(tree, "FTOF_layermult", &FTOF_layermult);
    }
};

/**
 * @brief Algorithm for AI PID hadron analysis using RICH detector
 *
 * Reads the RICH::Particle bank and matches entries to REC::Particle
 * to identify pions and kaons with RICH Cherenkov information.
 */
class AiPidHadronAlg : public clas12::framework::SimpleAlgorithm<AiPidHadronAlg> {
public:
    explicit AiPidHadronAlg(std::string name = "AiPidHadronAlg")
        : SimpleAlgorithm(std::move(name)) {
        declare_property(m_enable_histograms);
    }

    auto configure() -> void override {
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
        m_ntuple_svc = &optional_service<clas12::framework::NTupleService>("NTupleService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        if (m_enable_histograms && m_hist_svc && m_hist_svc->is_valid()) {
            book_histograms("/AiPidHadron/PiPlus",  h_piplus,  "#pi^{+}");
            book_histograms("/AiPidHadron/PiMinus", h_piminus, "#pi^{-}");
            book_histograms("/AiPidHadron/KPlus",   h_kplus,   "K^{+}");
            book_histograms("/AiPidHadron/KMinus",  h_kminus,  "K^{-}");
        }

        if (m_ntuple_svc && m_ntuple_svc->is_valid()) {
            m_ntuple.init(m_ntuple_svc->get(), "rich_hadrons");
        }

        // Initialize cutflows
        auto init_cutflow = [](clas12::framework::Cutflow& cf) {
            cf.add_cut("RICH hadrons");
            cf.add_cut("RICH match");
            cf.add_cut("RICH PID match");
            cf.add_cut("RQ > 0.1");
            cf.add_cut("best_ntot > 2.5");
        };
        init_cutflow(m_cutflow_piplus);
        init_cutflow(m_cutflow_piminus);
        init_cutflow(m_cutflow_kplus);
        init_cutflow(m_cutflow_kminus);

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        auto& hipo_event = event.hipo_event();
        auto [REC_Particle, RICH_Particle, REC_Calorimeter, REC_CaloExtras,
              REC_Cherenkov, REC_Scintillator, REC_ScintExtras] =
            hipo_event.banks("REC::Particle", "RICH::Particle",
                             "REC::Calorimeter", "REC::CaloExtras",
                             "REC::Cherenkov", "REC::Scintillator", "REC::ScintExtras");

        if (RICH_Particle->rows() == 0) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Extract all charged hadrons from REC::Particle
        auto particles = m_particle_svc->get()->extract_particles(
            *REC_Particle,
            {clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS,
             clas12::PIDs::KPLUS, clas12::PIDs::KMINUS});

        // Loop over all hadron candidates and look up their RICH info
        for (const auto& [pid_type, particle_vec] : particles) {
            for (const auto& particle : particle_vec) {
                // Determine cutflow based on REC PID
                clas12::framework::Cutflow* cf = nullptr;
                if (pid_type == static_cast<int>(clas12::PIDs::PIPLUS))       cf = &m_cutflow_piplus;
                else if (pid_type == static_cast<int>(clas12::PIDs::PIMINUS)) cf = &m_cutflow_piminus;
                else if (pid_type == static_cast<int>(clas12::PIDs::KPLUS))   cf = &m_cutflow_kplus;
                else if (pid_type == static_cast<int>(clas12::PIDs::KMINUS))  cf = &m_cutflow_kminus;

                if (cf) cf->pass("RICH hadrons");

                auto rich = clas12::read_RICHParticle_bank(*RICH_Particle, particle.index());

                // Skip particles without RICH match
                if (std::isnan(rich.best_ch)) continue;
                if (cf) cf->pass("RICH match");

                // Determine histogram set based on RICH best_PID
                int best_pid = rich.best_PID;
                HadronHistograms* h = nullptr;

                if (best_pid == static_cast<int>(clas12::PIDs::PIPLUS)) {
                    h = &h_piplus;
                } else if (best_pid == static_cast<int>(clas12::PIDs::PIMINUS)) {
                    h = &h_piminus;
                } else if (best_pid == static_cast<int>(clas12::PIDs::KPLUS)) {
                    h = &h_kplus;
                } else if (best_pid == static_cast<int>(clas12::PIDs::KMINUS)) {
                    h = &h_kminus;
                }

                if (!h) continue;
                if (cf) cf->pass("RICH PID match");

                // Fill pre-cut histograms
                fill_precut_histograms(particle, rich, *h);

                // Apply quality cuts
                if (rich.RQ <= 0.1f) continue;
                if (cf) cf->pass("RQ > 0.1");

                if (rich.best_ntot <= 2.5f) continue;
                if (cf) cf->pass("best_ntot > 2.5");

                // Fill post-cut histograms
                fill_postcut_histograms(particle, rich, *h);

                // Fill TTree
                if (m_ntuple) {
                    auto calo = clas12::read_Calorimeter_bank(*REC_Calorimeter, *REC_CaloExtras, particle.index());
                    auto htcc = clas12::read_Cherenkov_bank(*REC_Cherenkov, particle.index());
                    auto ftof = clas12::read_Scintillator_bank(*REC_Scintillator, *REC_ScintExtras, particle.index());

                    populate_particle_data(particle, rich);
                    populate_calorimeter_data(calo);
                    populate_cherenkov_data(htcc);
                    populate_scintillator_data(ftof);
                    m_ntuple.fill();
                }
            }
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        if (m_ntuple) {
            clas12::framework::Log::info(name(), "N-tuple entries: {}", m_ntuple.service()->get_entries(m_ntuple.tree_name()));
        }

        m_cutflow_piplus.print();
        m_cutflow_piminus.print();
        m_cutflow_kplus.print();
        m_cutflow_kminus.print();

        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::Property<bool> m_enable_histograms{
        "enable_histograms", true, "Enable histogram filling"
    };

    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;

    // N-tuple writer
    clas12::framework::NTupleWriter<HadronTreeData> m_ntuple;

    // Cutflows
    clas12::framework::Cutflow m_cutflow_piplus{"π⁺ RICH Selection"};
    clas12::framework::Cutflow m_cutflow_piminus{"π⁻ RICH Selection"};
    clas12::framework::Cutflow m_cutflow_kplus{"K⁺ RICH Selection"};
    clas12::framework::Cutflow m_cutflow_kminus{"K⁻ RICH Selection"};

    // --- TTree populate helpers ---

    void populate_particle_data(const clas12::Particle& particle, const clas12::RICHParticleBank& rich) {
        auto& d = m_ntuple.data();
        d.rich_pid = rich.best_PID;
        d.pid      = particle.pdg();
        d.p        = static_cast<float>(particle.p());
        d.theta    = static_cast<float>(particle.theta());
        d.phi      = static_cast<float>(particle.phi());
        d.beta     = static_cast<float>(particle.beta());
        d.charge   = particle.charge();
    }

    void populate_calorimeter_data(const clas12::CalorimeterBank& calo) {
        auto& d = m_ntuple.data();

        // Helper to populate one layer's fields
        auto fill_layer = [](auto& d_E, auto& d_chi2, auto& d_time, auto& d_path,
                             auto& d_lu, auto& d_lv, auto& d_lw,
                             auto& d_du, auto& d_dv, auto& d_dw,
                             auto& d_m2u, auto& d_m2v, auto& d_m2w,
                             auto& d_m3u, auto& d_m3v, auto& d_m3w,
                             auto& d_dbstU, auto& d_dbstV, auto& d_dbstW,
                             auto& d_rawEU, auto& d_rawEV, auto& d_rawEW,
                             auto& d_recEU, auto& d_recEV, auto& d_recEW,
                             auto& d_recDTU, auto& d_recDTV, auto& d_recDTW,
                             auto& d_recFTU, auto& d_recFTV, auto& d_recFTW,
                             auto& d_size,
                             const clas12::CalorimeterBank::CalorimeterStruct& layer) {
            d_E     = layer.energy;
            d_chi2  = layer.chi2;
            d_time  = layer.time;
            d_path  = layer.path;
            d_lu    = layer.lu;
            d_lv    = layer.lv;
            d_lw    = layer.lw;
            d_du    = layer.du;
            d_dv    = layer.dv;
            d_dw    = layer.dw;
            d_m2u   = layer.m2u;
            d_m2v   = layer.m2v;
            d_m2w   = layer.m2w;
            d_m3u   = layer.m3u;
            d_m3v   = layer.m3v;
            d_m3w   = layer.m3w;
            d_dbstU = layer.dbstU;
            d_dbstV = layer.dbstV;
            d_dbstW = layer.dbstW;
            d_rawEU = layer.rawEU;
            d_rawEV = layer.rawEV;
            d_rawEW = layer.rawEW;
            d_recEU = layer.recEU;
            d_recEV = layer.recEV;
            d_recEW = layer.recEW;
            d_recDTU = layer.recDTU;
            d_recDTV = layer.recDTV;
            d_recDTW = layer.recDTW;
            d_recFTU = layer.recFTU;
            d_recFTV = layer.recFTV;
            d_recFTW = layer.recFTW;
            d_size   = layer.size;
        };

        // PCAL
        fill_layer(d.PCAL_E, d.PCAL_chi2, d.PCAL_time, d.PCAL_path,
                   d.PCAL_lu, d.PCAL_lv, d.PCAL_lw,
                   d.PCAL_du, d.PCAL_dv, d.PCAL_dw,
                   d.PCAL_m2u, d.PCAL_m2v, d.PCAL_m2w,
                   d.PCAL_m3u, d.PCAL_m3v, d.PCAL_m3w,
                   d.PCAL_dbstU, d.PCAL_dbstV, d.PCAL_dbstW,
                   d.PCAL_rawEU, d.PCAL_rawEV, d.PCAL_rawEW,
                   d.PCAL_recEU, d.PCAL_recEV, d.PCAL_recEW,
                   d.PCAL_recDTU, d.PCAL_recDTV, d.PCAL_recDTW,
                   d.PCAL_recFTU, d.PCAL_recFTV, d.PCAL_recFTW,
                   d.PCAL_size, calo.pcal);

        // ECIN
        fill_layer(d.ICAL_E, d.ICAL_chi2, d.ICAL_time, d.ICAL_path,
                   d.ICAL_lu, d.ICAL_lv, d.ICAL_lw,
                   d.ICAL_du, d.ICAL_dv, d.ICAL_dw,
                   d.ICAL_m2u, d.ICAL_m2v, d.ICAL_m2w,
                   d.ICAL_m3u, d.ICAL_m3v, d.ICAL_m3w,
                   d.ICAL_dbstU, d.ICAL_dbstV, d.ICAL_dbstW,
                   d.ICAL_rawEU, d.ICAL_rawEV, d.ICAL_rawEW,
                   d.ICAL_recEU, d.ICAL_recEV, d.ICAL_recEW,
                   d.ICAL_recDTU, d.ICAL_recDTV, d.ICAL_recDTW,
                   d.ICAL_recFTU, d.ICAL_recFTV, d.ICAL_recFTW,
                   d.ICAL_size, calo.inner);

        // ECOUT
        fill_layer(d.OCAL_E, d.OCAL_chi2, d.OCAL_time, d.OCAL_path,
                   d.OCAL_lu, d.OCAL_lv, d.OCAL_lw,
                   d.OCAL_du, d.OCAL_dv, d.OCAL_dw,
                   d.OCAL_m2u, d.OCAL_m2v, d.OCAL_m2w,
                   d.OCAL_m3u, d.OCAL_m3v, d.OCAL_m3w,
                   d.OCAL_dbstU, d.OCAL_dbstV, d.OCAL_dbstW,
                   d.OCAL_rawEU, d.OCAL_rawEV, d.OCAL_rawEW,
                   d.OCAL_recEU, d.OCAL_recEV, d.OCAL_recEW,
                   d.OCAL_recDTU, d.OCAL_recDTV, d.OCAL_recDTW,
                   d.OCAL_recFTU, d.OCAL_recFTV, d.OCAL_recFTW,
                   d.OCAL_size, calo.outer);
    }

    void populate_cherenkov_data(const clas12::CherenkovBank& htcc) {
        auto& d = m_ntuple.data();
        d.HTCC_nphe   = htcc.nphe;
        d.HTCC_time   = htcc.time;
        d.HTCC_path   = htcc.path;
        d.HTCC_chi2   = htcc.chi2;
        d.HTCC_dtheta = htcc.dtheta;
        d.HTCC_dphi   = htcc.dphi;
    }

    void populate_scintillator_data(const clas12::ScintillatorBank& scint) {
        auto& d = m_ntuple.data();
        d.FTOF_E         = scint.energy;
        d.FTOF_time      = scint.time;
        d.FTOF_path      = scint.path;
        d.FTOF_chi2      = scint.chi2;
        d.FTOF_dedx      = scint.dedx;
        d.FTOF_size      = scint.size;
        d.FTOF_layermult = scint.layermult;
    }

    // --- Histograms ---

    struct HadronHistograms {
        struct {
            clas12::framework::AtomicHistogram1D* p = nullptr;
            clas12::framework::AtomicHistogram1D* theta = nullptr;
            clas12::framework::AtomicHistogram1D* phi = nullptr;
            clas12::framework::AtomicHistogram1D* vz = nullptr;
            clas12::framework::AtomicHistogram1D* RQ = nullptr;
            clas12::framework::AtomicHistogram1D* best_PID = nullptr;
        } precut;

        struct {
            clas12::framework::AtomicHistogram1D* p = nullptr;
            clas12::framework::AtomicHistogram1D* theta = nullptr;
            clas12::framework::AtomicHistogram1D* phi = nullptr;
            clas12::framework::AtomicHistogram1D* vz = nullptr;
            clas12::framework::AtomicHistogram1D* RQ = nullptr;
            clas12::framework::AtomicHistogram1D* best_PID = nullptr;
        } postcut;
    };

    HadronHistograms h_piplus;
    HadronHistograms h_piminus;
    HadronHistograms h_kplus;
    HadronHistograms h_kminus;

    void book_histograms(const std::string& dir, HadronHistograms& h, const std::string& label) {
        auto* svc = m_hist_svc->get();

        auto pre = dir + "/precut";
        h.precut.p        = svc->book_atomic_hist1D(pre + "/p",        fmt::format("{} Momentum (pre-cut);p [GeV/c];Events", label),    100, 0, 10);
        h.precut.theta    = svc->book_atomic_hist1D(pre + "/theta",    fmt::format("{} #theta (pre-cut);#theta [deg];Events", label),   180, 0, 40);
        h.precut.phi      = svc->book_atomic_hist1D(pre + "/phi",      fmt::format("{} #phi (pre-cut);#phi [deg];Events", label),       360, -180, 180);
        h.precut.vz       = svc->book_atomic_hist1D(pre + "/vz",       fmt::format("{} v_{{z}} (pre-cut);v_{{z}} [cm];Events", label),  200, -40, 40);
        h.precut.RQ       = svc->book_atomic_hist1D(pre + "/RQ",       fmt::format("{} RICH RQ (pre-cut);RQ;Events", label),            100, 0, 1);
        h.precut.best_PID = svc->book_atomic_hist1D(pre + "/best_PID", fmt::format("{} RICH best PID (pre-cut);PID;Events", label),     800, -400, 400);

        auto post = dir + "/postcut";
        h.postcut.p        = svc->book_atomic_hist1D(post + "/p",        fmt::format("{} Momentum (post-cut);p [GeV/c];Events", label),    100, 0, 10);
        h.postcut.theta    = svc->book_atomic_hist1D(post + "/theta",    fmt::format("{} #theta (post-cut);#theta [deg];Events", label),   180, 0, 40);
        h.postcut.phi      = svc->book_atomic_hist1D(post + "/phi",      fmt::format("{} #phi (post-cut);#phi [deg];Events", label),       360, -180, 180);
        h.postcut.vz       = svc->book_atomic_hist1D(post + "/vz",       fmt::format("{} v_{{z}} (post-cut);v_{{z}} [cm];Events", label),  200, -40, 40);
        h.postcut.RQ       = svc->book_atomic_hist1D(post + "/RQ",       fmt::format("{} RICH RQ (post-cut);RQ;Events", label),            100, 0, 1);
        h.postcut.best_PID = svc->book_atomic_hist1D(post + "/best_PID", fmt::format("{} RICH best PID (post-cut);PID;Events", label),     800, -400, 400);
    }

    void fill_precut_histograms(const clas12::Particle& particle, const clas12::RICHParticleBank& rich, HadronHistograms& h) {
        if (h.precut.p)        h.precut.p->fill(particle.p());
        if (h.precut.theta)    h.precut.theta->fill(particle.theta());
        if (h.precut.phi)      h.precut.phi->fill(particle.phi());
        if (h.precut.vz)       h.precut.vz->fill(particle.vz());
        if (h.precut.RQ)       h.precut.RQ->fill(rich.RQ);
        if (h.precut.best_PID) h.precut.best_PID->fill(rich.best_PID);
    }

    void fill_postcut_histograms(const clas12::Particle& particle, const clas12::RICHParticleBank& rich, HadronHistograms& h) {
        if (h.postcut.p)        h.postcut.p->fill(particle.p());
        if (h.postcut.theta)    h.postcut.theta->fill(particle.theta());
        if (h.postcut.phi)      h.postcut.phi->fill(particle.phi());
        if (h.postcut.vz)       h.postcut.vz->fill(particle.vz());
        if (h.postcut.RQ)       h.postcut.RQ->fill(rich.RQ);
        if (h.postcut.best_PID) h.postcut.best_PID->fill(rich.best_PID);
    }
};

}  // namespace ai_pid_hadron
