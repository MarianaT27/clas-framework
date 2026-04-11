#pragma once

/**
 * @file AiPidHadronAlg.hpp
 * @brief Algorithm for AI PID hadron analysis using RICH detector or exclusive reactions
 *
 * Supports two truth PID modes:
 * - "rich": Uses RICH::Particle best_PID as truth (default)
 * - "exclusive": Uses ed -> ep(n)pi+pi- exclusive reaction topology
 *
 * Outputs a ROOT TTree with particle, calorimeter, cherenkov,
 * and scintillator data for AI PID training.
 */

#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>
#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/physics/Kinematics.hpp>

#include <Math/Vector4D.h>
#include <Math/Vector3D.h>
#include <Math/VectorUtil.h>

#include <array>
#include <cmath>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <fmt/core.h>

namespace ai_pid_hadron {

/**
 * @brief N-Tuple data structure for hadron analysis
 *
 * Mirrors the TreeData fields from the AI PID reader,
 * with rich_pid replacing mc_pid.
 */
struct HadronTreeData {
    // Particle identification
    int rich_pid = 0;       // RICH best_PID (populated in "rich" mode, 0 in "exclusive" mode)
    int pid = 0;            // REC::Particle PID
    int truth_pid = 0;      // Mode-agnostic truth PID (RICH or topology)
    int truth_source = 0;   // 0 = RICH, 1 = exclusive reaction
    float missing_mass = 0; // Missing mass of exclusive reaction (0 in RICH mode)
    float missing_energy = 0;
    float missing_px = 0, missing_py = 0, missing_pz = 0;
    float missing_p = 0;
    float missing_pt = 0;
    float missing_theta = 0;
    float missing_phi = 0;
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
        svc->register_branch(tree, "truth_pid", &truth_pid);
        svc->register_branch(tree, "truth_source", &truth_source);
        svc->register_branch(tree, "missing_mass", &missing_mass);
        svc->register_branch(tree, "missing_energy", &missing_energy);
        svc->register_branch(tree, "missing_px", &missing_px);
        svc->register_branch(tree, "missing_py", &missing_py);
        svc->register_branch(tree, "missing_pz", &missing_pz);
        svc->register_branch(tree, "missing_p", &missing_p);
        svc->register_branch(tree, "missing_pt", &missing_pt);
        svc->register_branch(tree, "missing_theta", &missing_theta);
        svc->register_branch(tree, "missing_phi", &missing_phi);
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
 * @brief Algorithm for AI PID hadron analysis
 *
 * Supports two truth PID modes:
 * - "rich": Uses RICH::Particle best_PID as truth label
 * - "exclusive": Uses ed -> ep(n)pi+pi- reaction topology as truth label
 */
class AiPidHadronAlg : public clas12::framework::SimpleAlgorithm {
public:
    explicit AiPidHadronAlg(std::string name = "AiPidHadronAlg",
                            std::string truth_mode = "rich")
        : SimpleAlgorithm(std::move(name)) {
        declare_property(m_enable_histograms);
        declare_property(m_enable_ntuple);
        declare_property(m_truth_mode);
        declare_property(m_beam_energy);
        declare_property(m_mm_center);
        declare_property(m_mm_width);

        if (!truth_mode.empty()) {
            m_truth_mode.set(std::move(truth_mode));
        }
        // Declare required banks
        declare_bank("REC::Particle");
        declare_bank("RICH::Particle");
        declare_bank("REC::Calorimeter");
        declare_bank("REC::CaloExtras");
        declare_bank("REC::Cherenkov");
        declare_bank("REC::Scintillator");
        declare_bank("REC::ScintExtras");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_hist = svc<clas12::framework::HistogramService>();
        m_particle = svc<clas12::framework::ParticleSelectorService>();
        m_ntuple_svc = svc<clas12::framework::NTupleService>();

        clas12::framework::Log::info(name(), "Truth PID mode: {}", m_truth_mode.get());

        if (m_truth_mode.get() == "rich") {
            if (m_enable_histograms && m_hist) {
                book_histograms("/AiPidHadron/PiPlus",  h_piplus,  "#pi^{+}");
                book_histograms("/AiPidHadron/PiMinus", h_piminus, "#pi^{-}");
                book_histograms("/AiPidHadron/KPlus",   h_kplus,   "K^{+}");
                book_histograms("/AiPidHadron/KMinus",  h_kminus,  "K^{-}");

                // RICH vs REC PID comparison
                book_comparison_histograms();
            }

            auto init_cutflow = [](clas12::framework::Cutflow& cf) {
                cf.add_cut("RICH hadrons");
                cf.add_cut("RICH PID match");
                cf.add_cut("RQ > 0.1");
                cf.add_cut("best_ntot > 2.5");
            };
            init_cutflow(m_cutflow_piplus);
            init_cutflow(m_cutflow_piminus);
            init_cutflow(m_cutflow_kplus);
            init_cutflow(m_cutflow_kminus);
        } else if (m_truth_mode.get() == "exclusive") {
            clas12::framework::Log::info(name(), "Exclusive mode: ed -> ep(n)pi+pi-");
            clas12::framework::Log::info(name(), "  Beam energy: {} GeV", m_beam_energy.get());
            clas12::framework::Log::info(name(), "  MM cut: {} +/- {} GeV", m_mm_center.get(), m_mm_width.get());

            if (m_enable_histograms && m_hist) {
                book_exclusive_histograms();
            }

            m_cutflow_exclusive.add_cut("Total events");
            m_cutflow_exclusive.add_cut("Has good electron");
            m_cutflow_exclusive.add_cut("Passes DIS cuts");
            m_cutflow_exclusive.add_cut("Has >= 1 proton");
            m_cutflow_exclusive.add_cut("Has meson pair");
            m_cutflow_exclusive.add_cut("Missing mass cut");
        } else {
            clas12::framework::Log::error(name(), "Unknown truth_mode '{}'. Use 'rich' or 'exclusive'.", m_truth_mode.get());
            return clas12::framework::StatusCode::FAILURE();
        }

        if (m_enable_ntuple && m_ntuple_svc) {
            m_ntuple.init(m_ntuple_svc, "rich_hadrons");
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        if (m_truth_mode.get() == "exclusive") {
            return process_exclusive(event);
        }
        return process_rich(event);
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        if (m_ntuple) {
            clas12::framework::Log::info(name(), "N-tuple entries: {}", m_ntuple.service()->get_entries(m_ntuple.tree_name()));
        }

        if (m_truth_mode.get() == "rich") {
            m_cutflow_piplus.print();
            m_cutflow_piminus.print();
            m_cutflow_kplus.print();
            m_cutflow_kminus.print();

            // Print RICH PID summary table
            {
                size_t total = 0;
                for (const auto& [pid, count] : m_rich_pid_counts) total += count;

                std::vector<std::vector<std::string>> rows;
                for (const auto& [pid, count] : m_rich_pid_counts) {
                    auto pid_enum = clas12::to_pid_enum(pid);
                    std::string pname = pid_enum ? clas12::pid_name(*pid_enum) : fmt::format("Unknown({})", pid);
                    double frac = total > 0 ? 100.0 * static_cast<double>(count) / static_cast<double>(total) : 0.0;
                    rows.push_back({fmt::format("{}", pid), pname, fmt::format("{}", count), fmt::format("{:.2f}%", frac)});
                }
                clas12::framework::Log::print_yield_summary_table(
                    "RICH best_PID Summary",
                    {"PID", "Name", "Count", "Fraction"},
                    rows, fmt::format("Total: {}", total));
            }

            // Print RICH vs REC PID agreement summary (Sector 1)
            {
                const std::array<std::pair<int, std::string>, 4> species = {{
                    {211, "pi+"}, {-211, "pi-"}, {321, "K+"}, {-321, "K-"}
                }};
                std::vector<std::vector<std::string>> rows;
                size_t total_agree = 0, total_disagree = 0;
                for (const auto& [spid, label] : species) {
                    auto it = m_pid_agreement.find(spid);
                    if (it == m_pid_agreement.end()) continue;
                    size_t ag = it->second.agree;
                    size_t dis = it->second.disagree;
                    size_t tot = ag + dis;
                    double rate = tot > 0 ? 100.0 * static_cast<double>(ag) / static_cast<double>(tot) : 0.0;
                    rows.push_back({label, fmt::format("{}", ag), fmt::format("{}", dis),
                                    fmt::format("{}", tot), fmt::format("{:.2f}%", rate)});
                    total_agree += ag;
                    total_disagree += dis;
                }
                size_t grand_total = total_agree + total_disagree;
                double grand_rate = grand_total > 0 ? 100.0 * static_cast<double>(total_agree)
                                                       / static_cast<double>(grand_total) : 0.0;
                clas12::framework::Log::print_yield_summary_table(
                    "RICH vs REC PID Agreement (Sector 1)",
                    {"REC PID", "Agree", "Disagree", "Total", "Rate"},
                    rows, fmt::format("Overall: {}/{} ({:.2f}%)", total_agree, grand_total, grand_rate));
            }
        } else if (m_truth_mode.get() == "exclusive") {
            m_cutflow_exclusive.print();

            // Print exclusive PID summary table
            {
                size_t total = 0;
                for (const auto& [pid, count] : m_excl_pid_counts) total += count;

                std::vector<std::vector<std::string>> rows;
                for (const auto& [pid, count] : m_excl_pid_counts) {
                    auto pid_enum = clas12::to_pid_enum(pid);
                    std::string pname = pid_enum ? clas12::pid_name(*pid_enum) : fmt::format("Unknown({})", pid);
                    double frac = total > 0 ? 100.0 * static_cast<double>(count) / static_cast<double>(total) : 0.0;
                    rows.push_back({fmt::format("{}", pid), pname, fmt::format("{}", count), fmt::format("{:.2f}%", frac)});
                }
                clas12::framework::Log::print_yield_summary_table(
                    "Exclusive PID Summary",
                    {"PID", "Name", "Count", "Fraction"},
                    rows, fmt::format("Total: {}", total));
            }
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    // --- RICH vs REC PID comparison structs ---
    struct KinematicHistograms {
        clas12::framework::AtomicHistogram1D* p = nullptr;
        clas12::framework::AtomicHistogram1D* theta = nullptr;
        clas12::framework::AtomicHistogram1D* phi = nullptr;
        clas12::framework::AtomicHistogram1D* beta = nullptr;
    };

    struct PidCompareHistograms {
        KinematicHistograms agree;
        KinematicHistograms disagree;
        clas12::framework::AtomicHistogram1D* rich_pid_dist = nullptr; // RICH PID distribution for this REC species
    };

    struct PidAgreement {
        size_t agree = 0;
        size_t disagree = 0;
    };

    static auto pid_to_bin_index(int pid) -> double {
        switch (pid) {
            case  211: return 0.5;
            case -211: return 1.5;
            case  321: return 2.5;
            case -321: return 3.5;
            default:   return 4.5;
        }
    }

    // Per-particle cut variable histograms (forward declaration for use in methods and structs)
    struct ParticleCutHistograms {
        clas12::framework::AtomicHistogram1D* chi2pid = nullptr;
        clas12::framework::AtomicHistogram1D* p = nullptr;
        clas12::framework::AtomicHistogram1D* vz = nullptr;
        clas12::framework::AtomicHistogram1D* delta_vz = nullptr;
    };

    // Exclusivity variables computed from four-momentum conservation
    struct ExclusivityVars {
        double missing_energy = 0;
        double missing_px = 0, missing_py = 0, missing_pz = 0;
        double missing_p = 0;
        double missing_pt = 0;
        double missing_theta = 0;
        double missing_phi = 0;
        double missing_mass = 0;
        double missing_mass2 = 0;
    };

    // Per-channel exclusivity histograms
    struct ExclusivityHistograms {
        clas12::framework::AtomicHistogram1D* missing_energy = nullptr;
        clas12::framework::AtomicHistogram1D* missing_p = nullptr;
        clas12::framework::AtomicHistogram1D* missing_pt = nullptr;
        clas12::framework::AtomicHistogram1D* missing_theta = nullptr;
        clas12::framework::AtomicHistogram1D* missing_phi = nullptr;
        // Detected neutron (filled only when neutron found)
        clas12::framework::AtomicHistogram1D* n_detected = nullptr;
        clas12::framework::AtomicHistogram1D* neutron_opening_angle = nullptr;
        clas12::framework::AtomicHistogram1D* neutron_p = nullptr;
        clas12::framework::AtomicHistogram1D* neutron_beta = nullptr;
        clas12::framework::AtomicHistogram1D* mm_with_neutron = nullptr; // MM(ed→epn+mesons)
    };

    // --- Properties ---
    clas12::framework::Property<bool> m_enable_histograms{
        "enable_histograms", true, "Enable histogram filling"
    };
    clas12::framework::Property<std::string> m_truth_mode{
        "truth_mode", "rich", "Truth PID mode: 'rich' or 'exclusive'"
    };
    clas12::framework::Property<double> m_beam_energy{
        "beam_energy", 10.53, "Beam energy [GeV]"
    };
    clas12::framework::Property<double> m_mm_center{
        "mm_center", 0.9396, "Missing mass cut center [GeV] (neutron mass)"
    };
    clas12::framework::Property<double> m_mm_width{
        "mm_width", 0.2, "Missing mass cut half-width [GeV]"
    };
    clas12::framework::Property<bool> m_enable_ntuple{
        "enable_ntuple", false, "Enable N-tuple output"
    };

    // --- Services ---
    clas12::framework::HistogramService* m_hist = nullptr;
    clas12::framework::ParticleSelectorService* m_particle = nullptr;
    clas12::framework::NTupleService* m_ntuple_svc = nullptr;

    // N-tuple writer
    clas12::framework::NTupleWriter<HadronTreeData> m_ntuple;

    // --- RICH mode members ---
    clas12::framework::Cutflow m_cutflow_piplus{"π⁺ RICH Selection"};
    clas12::framework::Cutflow m_cutflow_piminus{"π⁻ RICH Selection"};
    clas12::framework::Cutflow m_cutflow_kplus{"K⁺ RICH Selection"};
    clas12::framework::Cutflow m_cutflow_kminus{"K⁻ RICH Selection"};
    std::mutex m_pid_count_mutex;
    std::map<int, size_t> m_rich_pid_counts;
    std::map<int, PidAgreement> m_pid_agreement;

    // RICH vs REC PID comparison histograms (Sector 1 only)
    clas12::framework::AtomicHistogram2D* h_rich_vs_rec_pid = nullptr;
    PidCompareHistograms h_cmp_piplus;
    PidCompareHistograms h_cmp_piminus;
    PidCompareHistograms h_cmp_kplus;
    PidCompareHistograms h_cmp_kminus;

    // Per-RICH-species REC PID distribution (Sector 1)
    clas12::framework::AtomicHistogram1D* h_rec_pid_for_rich_piplus = nullptr;
    clas12::framework::AtomicHistogram1D* h_rec_pid_for_rich_piminus = nullptr;
    clas12::framework::AtomicHistogram1D* h_rec_pid_for_rich_kplus = nullptr;
    clas12::framework::AtomicHistogram1D* h_rec_pid_for_rich_kminus = nullptr;

    // --- Exclusive mode members ---
    clas12::framework::Cutflow m_cutflow_exclusive{"Exclusive ed→ep(n)π⁺π⁻"};
    std::mutex m_excl_count_mutex;
    std::map<int, size_t> m_excl_pid_counts;

    // ========================================================================
    // RICH mode processing
    // ========================================================================

    auto process_rich(clas12::framework::Event& event) -> clas12::framework::StatusCode {
        const auto& REC_Particle = event.get_bank("REC::Particle");
        // Every detector bank below is accessed via the per-event
        // generated-wrapper cache through the read_*_bank helpers; no
        // local hipo::bank_view handles are needed here.

        if (event.get_bank("RICH::Particle").rows() == 0) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Extract all charged hadrons from REC::Particle
        auto particles = m_particle->extract_particles(
            REC_Particle,
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

                auto rich = clas12::read_RICHParticle_bank(event, particle.index());

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

                // Count all RICH best_PIDs (before any cuts)
                int rec_pid = static_cast<int>(pid_type);
                {
                    std::lock_guard<std::mutex> lock(m_pid_count_mutex);
                    m_rich_pid_counts[best_pid]++;
                }

                // Determine particle sector from scintillator
                auto ftof = clas12::read_Scintillator_bank_with_extras(event, particle.index());
                int sector = ftof.sector;

                // Sector 1: track agreement and fill comparison histograms
                if (sector == 1) {
                    {
                        std::lock_guard<std::mutex> lock(m_pid_count_mutex);
                        if (best_pid == rec_pid) {
                            m_pid_agreement[rec_pid].agree++;
                        } else {
                            m_pid_agreement[rec_pid].disagree++;
                        }
                    }
                    if (m_enable_histograms) {
                        fill_comparison(particle, rec_pid, best_pid);
                    }
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
                    auto calo = clas12::read_Calorimeter_bank_with_extras(event, particle.index());
                    auto htcc = clas12::read_Cherenkov_bank(event, particle.index());
                    auto ftof = clas12::read_Scintillator_bank_with_extras(event, particle.index());

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

    // ========================================================================
    // Exclusive mode processing: ed -> ep(n)pi+pi-
    // ========================================================================

    auto process_exclusive(clas12::framework::Event& event) -> clas12::framework::StatusCode {
        m_cutflow_exclusive.pass("Total events");

        // 1. Require good DIS electron from upstream
        auto electron_opt = clas12::framework::EventHelpers::get_dis_electron(event);
        if (!electron_opt) return clas12::framework::StatusCode::FILTERED();
        const auto& electron = *electron_opt;
        m_cutflow_exclusive.pass("Has good electron");
        m_cutflow_exclusive.pass("Passes DIS cuts");

        // 2. Extract hadrons from REC::Particle. Detector banks are read
        // through the per-event generated-wrapper cache inside the
        // fill_exclusive_ntuple helper, so no local bank-view handles
        // are needed here.
        const auto& REC_Particle = event.get_bank("REC::Particle");

        auto particles = m_particle->extract_particles(
            REC_Particle,
            {clas12::PIDs::PROTON, clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS,
             clas12::PIDs::KPLUS, clas12::PIDs::KMINUS, clas12::PIDs::NEUTRON});

        const auto& protons = m_particle->get_protons(particles);
        const auto& pions_plus = clas12::framework::ParticleSelectorService::get_pions_plus(particles);
        const auto& pions_minus = clas12::framework::ParticleSelectorService::get_pions_minus(particles);

        static const std::vector<clas12::Particle> empty;
        auto kp_it = particles.find(clas12::PIDs::KPLUS);
        auto km_it = particles.find(clas12::PIDs::KMINUS);
        auto n_it  = particles.find(clas12::PIDs::NEUTRON);
        const auto& kaons_plus  = (kp_it != particles.end()) ? kp_it->second : empty;
        const auto& kaons_minus = (km_it != particles.end()) ? km_it->second : empty;
        const auto& neutrons    = (n_it  != particles.end()) ? n_it->second  : empty;

        // 3. Topology check: require proton + (pi+pi- or K+K-)
        if (protons.empty()) return clas12::framework::StatusCode::FILTERED();
        m_cutflow_exclusive.pass("Has >= 1 proton");

        bool has_pion_pair = !pions_plus.empty() && !pions_minus.empty();
        bool has_kaon_pair = !kaons_plus.empty() && !kaons_minus.empty();

        if (!has_pion_pair && !has_kaon_pair) return clas12::framework::StatusCode::FILTERED();
        m_cutflow_exclusive.pass("Has meson pair");

        // 3b. Fill hadron cut variable histograms (before quality cuts)
        const double e_vz = electron.vz();
        if (m_enable_histograms) {
            fill_all_cut_histograms(e_vz, protons, pions_plus, pions_minus, kaons_plus, kaons_minus);
        }

        // 4. Construct beam and deuterium target
        const double beam_E = m_beam_energy.get();
        clas12::Particle beam(11, 0, -1, -1, clas12::Constants::ELECTRONMASS,
                              0, 0, beam_E, beam_E, 0, 0, 0, 0, 1.0, 0);

        clas12::Particle target(static_cast<int>(clas12::PIDs::DEUTERON), 0, -1, 0,
                                clas12::Constants::DEUTERIUMMASS,
                                0, 0, 0, clas12::Constants::DEUTERIUMMASS, 0, 0, 0, 0, 0, 0);

        // 5. Loop over all (p, pi+, pi-) combinations
        if (h_exclusive.n_combinations) {
            int n_combos = static_cast<int>(protons.size() * pions_plus.size() * pions_minus.size());
            h_exclusive.n_combinations->fill(n_combos);
        }

        bool any_passed_mm = false;

        for (const auto& proton : protons) {
            if (!passes_hadron_cuts(proton, e_vz)) continue;
            for (const auto& pip : pions_plus) {
                if (!passes_hadron_cuts(pip, e_vz)) continue;
                for (const auto& pim : pions_minus) {
                    if (!passes_hadron_cuts(pim, e_vz)) continue;

                    // Detected final state: e, p, pi+, pi-
                    std::vector<clas12::Particle> detected = {electron, proton, pip, pim};
                    auto excl = compute_exclusivity(beam, target, detected);

                    // Fill pre-cut missing mass histograms (without neutron)
                    if (h_exclusive.mm_all) h_exclusive.mm_all->fill(excl.missing_mass);
                    if (h_exclusive.mm_squared) h_exclusive.mm_squared->fill(excl.missing_mass2);

                    // Neutron detection
                    if (h_exclusive.pion_excl.n_detected) h_exclusive.pion_excl.n_detected->fill(static_cast<int>(neutrons.size()));
                    if (!neutrons.empty()) {
                        // Find neutron closest to missing momentum direction
                        ROOT::Math::XYZVector miss_dir(excl.missing_px, excl.missing_py, excl.missing_pz);
                        double best_angle = 999;
                        const clas12::Particle* best_n = nullptr;
                        for (const auto& n : neutrons) {
                            double angle = ROOT::Math::VectorUtil::Angle(miss_dir, n.PxPyPzVector()) * 180.0 / M_PI;
                            if (angle < best_angle) { best_angle = angle; best_n = &n; }
                        }
                        if (best_n) {
                            if (h_exclusive.pion_excl.neutron_opening_angle) h_exclusive.pion_excl.neutron_opening_angle->fill(best_angle);
                            if (h_exclusive.pion_excl.neutron_p)    h_exclusive.pion_excl.neutron_p->fill(best_n->p());
                            if (h_exclusive.pion_excl.neutron_beta) h_exclusive.pion_excl.neutron_beta->fill(best_n->beta());
                            // Exclusivity variables with neutron: ed → e p n π+ π- (X)
                            std::vector<clas12::Particle> detected_with_n = {electron, proton, pip, pim, *best_n};
                            auto excl_n = compute_exclusivity(beam, target, detected_with_n);
                            if (h_exclusive.pion_excl.mm_with_neutron) h_exclusive.pion_excl.mm_with_neutron->fill(excl_n.missing_mass);
                            if (h_exclusive.pion_excl.missing_energy)  h_exclusive.pion_excl.missing_energy->fill(excl_n.missing_energy);
                            if (h_exclusive.pion_excl.missing_p)       h_exclusive.pion_excl.missing_p->fill(excl_n.missing_p);
                            if (h_exclusive.pion_excl.missing_pt)      h_exclusive.pion_excl.missing_pt->fill(excl_n.missing_pt);
                            if (h_exclusive.pion_excl.missing_theta)   h_exclusive.pion_excl.missing_theta->fill(excl_n.missing_theta);
                            if (h_exclusive.pion_excl.missing_phi)     h_exclusive.pion_excl.missing_phi->fill(excl_n.missing_phi);
                        }
                    }

                    // Apply missing mass cut: |MM - neutron_mass| < width
                    if (std::abs(excl.missing_mass - m_mm_center.get()) > m_mm_width.get()) continue;

                    any_passed_mm = true;

                    // Fill post-cut histograms
                    if (h_exclusive.mm_selected) h_exclusive.mm_selected->fill(excl.missing_mass);
                    if (h_exclusive.piplus_p) h_exclusive.piplus_p->fill(pip.p());
                    if (h_exclusive.piplus_theta) h_exclusive.piplus_theta->fill(pip.theta());
                    if (h_exclusive.piminus_p) h_exclusive.piminus_p->fill(pim.p());
                    if (h_exclusive.piminus_theta) h_exclusive.piminus_theta->fill(pim.theta());

                    // Fill NTuple for pi+ (true PID = 211) and pi- (true PID = -211)
                    if (m_ntuple) {
                        fill_exclusive_ntuple(event, pip, static_cast<int>(clas12::PIDs::PIPLUS), excl);
                        fill_exclusive_ntuple(event, pim, static_cast<int>(clas12::PIDs::PIMINUS), excl);
                    }

                    // Count exclusive PIDs
                    {
                        std::lock_guard<std::mutex> lock(m_excl_count_mutex);
                        m_excl_pid_counts[static_cast<int>(clas12::PIDs::PIPLUS)]++;
                        m_excl_pid_counts[static_cast<int>(clas12::PIDs::PIMINUS)]++;
                    }
                }
            }
        }

        // 6. Loop over all (p, K+, K-) combinations
        if (h_exclusive.kaon_n_combinations) {
            int n_combos = static_cast<int>(protons.size() * kaons_plus.size() * kaons_minus.size());
            h_exclusive.kaon_n_combinations->fill(n_combos);
        }

        for (const auto& proton : protons) {
            if (!passes_hadron_cuts(proton, e_vz)) continue;
            for (const auto& kp : kaons_plus) {
                if (!passes_hadron_cuts(kp, e_vz)) continue;
                for (const auto& km : kaons_minus) {
                    if (!passes_hadron_cuts(km, e_vz)) continue;

                    std::vector<clas12::Particle> detected = {electron, proton, kp, km};
                    auto excl = compute_exclusivity(beam, target, detected);

                    // Fill pre-cut missing mass histograms (without neutron)
                    if (h_exclusive.kaon_mm_all) h_exclusive.kaon_mm_all->fill(excl.missing_mass);
                    if (h_exclusive.kaon_mm_squared) h_exclusive.kaon_mm_squared->fill(excl.missing_mass2);

                    // Neutron detection
                    if (h_exclusive.kaon_excl.n_detected) h_exclusive.kaon_excl.n_detected->fill(static_cast<int>(neutrons.size()));
                    if (!neutrons.empty()) {
                        ROOT::Math::XYZVector miss_dir(excl.missing_px, excl.missing_py, excl.missing_pz);
                        double best_angle = 999;
                        const clas12::Particle* best_n = nullptr;
                        for (const auto& n : neutrons) {
                            double angle = ROOT::Math::VectorUtil::Angle(miss_dir, n.PxPyPzVector()) * 180.0 / M_PI;
                            if (angle < best_angle) { best_angle = angle; best_n = &n; }
                        }
                        if (best_n) {
                            if (h_exclusive.kaon_excl.neutron_opening_angle) h_exclusive.kaon_excl.neutron_opening_angle->fill(best_angle);
                            if (h_exclusive.kaon_excl.neutron_p)    h_exclusive.kaon_excl.neutron_p->fill(best_n->p());
                            if (h_exclusive.kaon_excl.neutron_beta) h_exclusive.kaon_excl.neutron_beta->fill(best_n->beta());
                            // Exclusivity variables with neutron: ed → e p n K+ K- (X)
                            std::vector<clas12::Particle> detected_with_n = {electron, proton, kp, km, *best_n};
                            auto excl_n = compute_exclusivity(beam, target, detected_with_n);
                            if (h_exclusive.kaon_excl.mm_with_neutron) h_exclusive.kaon_excl.mm_with_neutron->fill(excl_n.missing_mass);
                            if (h_exclusive.kaon_excl.missing_energy)  h_exclusive.kaon_excl.missing_energy->fill(excl_n.missing_energy);
                            if (h_exclusive.kaon_excl.missing_p)       h_exclusive.kaon_excl.missing_p->fill(excl_n.missing_p);
                            if (h_exclusive.kaon_excl.missing_pt)      h_exclusive.kaon_excl.missing_pt->fill(excl_n.missing_pt);
                            if (h_exclusive.kaon_excl.missing_theta)   h_exclusive.kaon_excl.missing_theta->fill(excl_n.missing_theta);
                            if (h_exclusive.kaon_excl.missing_phi)     h_exclusive.kaon_excl.missing_phi->fill(excl_n.missing_phi);
                        }
                    }

                    if (std::abs(excl.missing_mass - m_mm_center.get()) > m_mm_width.get()) continue;

                    any_passed_mm = true;

                    if (h_exclusive.kaon_mm_selected) h_exclusive.kaon_mm_selected->fill(excl.missing_mass);
                    if (h_exclusive.kplus_p) h_exclusive.kplus_p->fill(kp.p());
                    if (h_exclusive.kplus_theta) h_exclusive.kplus_theta->fill(kp.theta());
                    if (h_exclusive.kminus_p) h_exclusive.kminus_p->fill(km.p());
                    if (h_exclusive.kminus_theta) h_exclusive.kminus_theta->fill(km.theta());

                    if (m_ntuple) {
                        fill_exclusive_ntuple(event, kp, static_cast<int>(clas12::PIDs::KPLUS), excl);
                        fill_exclusive_ntuple(event, km, static_cast<int>(clas12::PIDs::KMINUS), excl);
                    }

                    {
                        std::lock_guard<std::mutex> lock(m_excl_count_mutex);
                        m_excl_pid_counts[static_cast<int>(clas12::PIDs::KPLUS)]++;
                        m_excl_pid_counts[static_cast<int>(clas12::PIDs::KMINUS)]++;
                    }
                }
            }
        }

        if (any_passed_mm) {
            m_cutflow_exclusive.pass("Missing mass cut");
        }

        return any_passed_mm ? clas12::framework::StatusCode::SUCCESS()
                             : clas12::framework::StatusCode::FILTERED();
    }

    // ========================================================================
    // Hadron quality cuts
    // ========================================================================

    void fill_cut_histograms(ParticleCutHistograms& h, const clas12::Particle& p, double electron_vz) {
        if (h.chi2pid)  h.chi2pid->fill(p.chi2pid());
        if (h.p)        h.p->fill(p.p());
        if (h.vz)       h.vz->fill(p.vz());
        if (h.delta_vz) h.delta_vz->fill(p.vz() - electron_vz);
    }

    void fill_all_cut_histograms(double electron_vz,
                                  const std::vector<clas12::Particle>& protons,
                                  const std::vector<clas12::Particle>& pions_plus,
                                  const std::vector<clas12::Particle>& pions_minus,
                                  const std::vector<clas12::Particle>& kaons_plus,
                                  const std::vector<clas12::Particle>& kaons_minus) {
        for (const auto& p : protons)      fill_cut_histograms(h_exclusive.proton_cuts,  p, electron_vz);
        for (const auto& p : pions_plus)   fill_cut_histograms(h_exclusive.piplus_cuts,   p, electron_vz);
        for (const auto& p : pions_minus)  fill_cut_histograms(h_exclusive.piminus_cuts,  p, electron_vz);
        for (const auto& p : kaons_plus)   fill_cut_histograms(h_exclusive.kplus_cuts,    p, electron_vz);
        for (const auto& p : kaons_minus)  fill_cut_histograms(h_exclusive.kminus_cuts,   p, electron_vz);
    }

    bool passes_hadron_cuts(const clas12::Particle& hadron, double electron_vz) const {
        if (hadron.p() < 0.3) return false;
        if (hadron.chi2pid() < -3.0 || hadron.chi2pid() > 3.0) return false;
        if (hadron.vz() < -15.0 || hadron.vz() > 5.0) return false;
        if (std::abs(hadron.vz() - electron_vz) > 3.0) return false;
        return true;
    }

    static auto compute_exclusivity(const clas12::Particle& beam,
                                     const clas12::Particle& target,
                                     const std::vector<clas12::Particle>& detected) -> ExclusivityVars {
        auto p_miss = beam.PxPyPzEVector() + target.PxPyPzEVector();
        for (const auto& p : detected) {
            p_miss -= p.PxPyPzEVector();
        }

        ExclusivityVars vars;
        vars.missing_px     = p_miss.Px();
        vars.missing_py     = p_miss.Py();
        vars.missing_pz     = p_miss.Pz();
        vars.missing_energy = p_miss.E();
        vars.missing_p      = p_miss.P();
        vars.missing_pt     = p_miss.Pt();
        vars.missing_theta  = p_miss.Theta() * 180.0 / M_PI;
        vars.missing_phi    = p_miss.Phi()   * 180.0 / M_PI;
        vars.missing_mass2  = p_miss.M2();
        vars.missing_mass   = (vars.missing_mass2 > 0) ? std::sqrt(vars.missing_mass2)
                                                        : -std::sqrt(-vars.missing_mass2);
        return vars;
    }

    // ========================================================================
    // TTree populate helpers
    // ========================================================================

    void populate_particle_data(const clas12::Particle& particle, const clas12::RICHParticleBank& rich) {
        auto& d = m_ntuple.data();
        d.rich_pid     = rich.best_PID;
        d.truth_pid    = rich.best_PID;
        d.truth_source = 0;
        d.missing_mass = 0;
        d.pid          = particle.pdg();
        d.p            = static_cast<float>(particle.p());
        d.theta        = static_cast<float>(particle.theta() * M_PI / 180.0); // Convert to radians
        d.phi          = static_cast<float>(particle.phi() * M_PI / 180.0); // Convert to radians
        d.beta         = static_cast<float>(particle.beta());
        d.charge       = particle.charge();
    }

    void fill_exclusive_ntuple(clas12::framework::Event& event,
                               const clas12::Particle& hadron, int true_pid,
                               const ExclusivityVars& excl) {
        auto calo = clas12::read_Calorimeter_bank_with_extras(event, hadron.index());
        auto htcc = clas12::read_Cherenkov_bank(event, hadron.index());
        auto ftof = clas12::read_Scintillator_bank_with_extras(event, hadron.index());

        auto& d = m_ntuple.data();
        d.rich_pid       = 0;
        d.truth_pid      = true_pid;
        d.truth_source   = 1;
        d.missing_mass   = static_cast<float>(excl.missing_mass);
        d.missing_energy = static_cast<float>(excl.missing_energy);
        d.missing_px     = static_cast<float>(excl.missing_px);
        d.missing_py     = static_cast<float>(excl.missing_py);
        d.missing_pz     = static_cast<float>(excl.missing_pz);
        d.missing_p      = static_cast<float>(excl.missing_p);
        d.missing_pt     = static_cast<float>(excl.missing_pt);
        d.missing_theta  = static_cast<float>(excl.missing_theta);
        d.missing_phi    = static_cast<float>(excl.missing_phi);
        d.pid            = hadron.pdg();
        d.p              = static_cast<float>(hadron.p());
        d.theta          = static_cast<float>(hadron.theta());
        d.phi            = static_cast<float>(hadron.phi());
        d.beta           = static_cast<float>(hadron.beta());
        d.charge         = hadron.charge();

        populate_calorimeter_data(calo);
        populate_cherenkov_data(htcc);
        populate_scintillator_data(ftof);
        m_ntuple.fill();
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

    // ========================================================================
    // Histograms
    // ========================================================================

    // --- RICH mode histograms ---
    struct HadronHistograms {
        struct {
            clas12::framework::AtomicHistogram1D* p = nullptr;
            clas12::framework::AtomicHistogram1D* theta = nullptr;
            clas12::framework::AtomicHistogram1D* phi = nullptr;
            clas12::framework::AtomicHistogram1D* vz = nullptr;
            clas12::framework::AtomicHistogram1D* RQ = nullptr;
            clas12::framework::AtomicHistogram1D* best_PID = nullptr;
            clas12::framework::AtomicHistogram1D* best_ntot = nullptr;
        } precut;

        struct {
            clas12::framework::AtomicHistogram1D* p = nullptr;
            clas12::framework::AtomicHistogram1D* theta = nullptr;
            clas12::framework::AtomicHistogram1D* phi = nullptr;
            clas12::framework::AtomicHistogram1D* vz = nullptr;
            clas12::framework::AtomicHistogram1D* RQ = nullptr;
            clas12::framework::AtomicHistogram1D* best_PID = nullptr;
            clas12::framework::AtomicHistogram1D* best_ntot = nullptr;
        } postcut;
    };

    HadronHistograms h_piplus;
    HadronHistograms h_piminus;
    HadronHistograms h_kplus;
    HadronHistograms h_kminus;

    void book_comparison_histograms() {
        auto* svc = m_hist;
        auto dir = std::string("/AiPidHadron/RICHComparison");

        h_rich_vs_rec_pid = svc->book_hist2D(
            dir + "/rec_vs_rich_pid",
            "REC PID vs RICH best_PID;REC PID;RICH best_PID",
            5, 0, 5, 5, 0, 5);

        auto book_cmp = [&](PidCompareHistograms& h, const std::string& base, const std::string& label) {
            h.rich_pid_dist = svc->book_atomic_hist1D(base + "/rich_pid_dist",
                fmt::format("RICH best_PID for REC {};RICH PID;Events", label), 800, -400, 400);

            auto book_kin = [&](KinematicHistograms& kh, const std::string& sub, const std::string& tag) {
                kh.p     = svc->book_atomic_hist1D(sub + "/p",     fmt::format("{} {} Momentum;p [GeV/c];Events", label, tag),       200, 0, 10);
                kh.theta = svc->book_atomic_hist1D(sub + "/theta", fmt::format("{} {} #theta;#theta [deg];Events", label, tag),      200, 0, 40);
                kh.phi   = svc->book_atomic_hist1D(sub + "/phi",   fmt::format("{} {} #phi;#phi [deg];Events", label, tag),          200, -180, 180);
                kh.beta  = svc->book_atomic_hist1D(sub + "/beta",  fmt::format("{} {} #beta;#beta;Events", label, tag),              200, 0, 1.2);
            };
            book_kin(h.agree,    base + "/agree",    "(RICH=REC)");
            book_kin(h.disagree, base + "/disagree", "(RICH#neqREC)");
        };

        book_cmp(h_cmp_piplus,  dir + "/PiPlus",  "#pi^{+} S1");
        book_cmp(h_cmp_piminus, dir + "/PiMinus", "#pi^{-} S1");
        book_cmp(h_cmp_kplus,   dir + "/KPlus",   "K^{+} S1");
        book_cmp(h_cmp_kminus,  dir + "/KMinus",  "K^{-} S1");

        // Per-RICH-species REC PID distribution
        auto rdir = dir + "/ByRICH";
        h_rec_pid_for_rich_piplus  = svc->book_atomic_hist1D(rdir + "/rec_pid_for_rich_piplus",
            "REC PID for RICH #pi^{+} S1;REC PID;Events", 800, -400, 400);
        h_rec_pid_for_rich_piminus = svc->book_atomic_hist1D(rdir + "/rec_pid_for_rich_piminus",
            "REC PID for RICH #pi^{-} S1;REC PID;Events", 800, -400, 400);
        h_rec_pid_for_rich_kplus   = svc->book_atomic_hist1D(rdir + "/rec_pid_for_rich_kplus",
            "REC PID for RICH K^{+} S1;REC PID;Events", 800, -400, 400);
        h_rec_pid_for_rich_kminus  = svc->book_atomic_hist1D(rdir + "/rec_pid_for_rich_kminus",
            "REC PID for RICH K^{-} S1;REC PID;Events", 800, -400, 400);
    }

    void fill_comparison(const clas12::Particle& particle, int rec_pid, int rich_pid) {
        PidCompareHistograms* h = nullptr;
        if      (rec_pid == static_cast<int>(clas12::PIDs::PIPLUS))  h = &h_cmp_piplus;
        else if (rec_pid == static_cast<int>(clas12::PIDs::PIMINUS)) h = &h_cmp_piminus;
        else if (rec_pid == static_cast<int>(clas12::PIDs::KPLUS))   h = &h_cmp_kplus;
        else if (rec_pid == static_cast<int>(clas12::PIDs::KMINUS))  h = &h_cmp_kminus;
        if (!h) return;

        if (h->rich_pid_dist) h->rich_pid_dist->fill(rich_pid);
        if (h_rich_vs_rec_pid) h_rich_vs_rec_pid->fill(pid_to_bin_index(rec_pid), pid_to_bin_index(rich_pid));

        // Fill REC PID distribution keyed by RICH species
        if      (rich_pid == static_cast<int>(clas12::PIDs::PIPLUS)  && h_rec_pid_for_rich_piplus)  h_rec_pid_for_rich_piplus->fill(rec_pid);
        else if (rich_pid == static_cast<int>(clas12::PIDs::PIMINUS) && h_rec_pid_for_rich_piminus) h_rec_pid_for_rich_piminus->fill(rec_pid);
        else if (rich_pid == static_cast<int>(clas12::PIDs::KPLUS)   && h_rec_pid_for_rich_kplus)   h_rec_pid_for_rich_kplus->fill(rec_pid);
        else if (rich_pid == static_cast<int>(clas12::PIDs::KMINUS)  && h_rec_pid_for_rich_kminus)  h_rec_pid_for_rich_kminus->fill(rec_pid);

        auto& kh = (rec_pid == rich_pid) ? h->agree : h->disagree;
        if (kh.p)     kh.p->fill(particle.p());
        if (kh.theta) kh.theta->fill(particle.theta());
        if (kh.phi)   kh.phi->fill(particle.phi());
        if (kh.beta)  kh.beta->fill(particle.beta());
    }

    void book_histograms(const std::string& dir, HadronHistograms& h, const std::string& label) {
        auto* svc = m_hist;

        auto pre = dir + "/precut";
        h.precut.p        = svc->book_atomic_hist1D(pre + "/p",        fmt::format("{} Momentum (pre-cut);p [GeV/c];Events", label),    200, 0, 10);
        h.precut.theta    = svc->book_atomic_hist1D(pre + "/theta",    fmt::format("{} #theta (pre-cut);#theta [deg];Events", label),   200, 0, 40);
        h.precut.phi      = svc->book_atomic_hist1D(pre + "/phi",      fmt::format("{} #phi (pre-cut);#phi [deg];Events", label),       200, -180, 180);
        h.precut.vz       = svc->book_atomic_hist1D(pre + "/vz",       fmt::format("{} v_{{z}} (pre-cut);v_{{z}} [cm];Events", label),  200, -30, 30);
        h.precut.RQ       = svc->book_atomic_hist1D(pre + "/RQ",       fmt::format("{} RICH RQ (pre-cut);RQ;Events", label),            200, 0, 1);
        h.precut.best_PID = svc->book_atomic_hist1D(pre + "/best_PID", fmt::format("{} RICH best PID (pre-cut);PID;Events", label),     800, -400, 400);
        h.precut.best_ntot = svc->book_atomic_hist1D(pre + "/best_ntot", fmt::format("{} RICH best ntot (pre-cut);ntot;Events", label),     800, -400, 400);

        auto post = dir + "/postcut";
        h.postcut.p        = svc->book_atomic_hist1D(post + "/p",        fmt::format("{} Momentum (post-cut);p [GeV/c];Events", label),    200, 0, 10);
        h.postcut.theta    = svc->book_atomic_hist1D(post + "/theta",    fmt::format("{} #theta (post-cut);#theta [deg];Events", label),   200, 0, 40);
        h.postcut.phi      = svc->book_atomic_hist1D(post + "/phi",      fmt::format("{} #phi (post-cut);#phi [deg];Events", label),       200, -180, 180);
        h.postcut.vz       = svc->book_atomic_hist1D(post + "/vz",       fmt::format("{} v_{{z}} (post-cut);v_{{z}} [cm];Events", label),  200, -30, 30);
        h.postcut.RQ       = svc->book_atomic_hist1D(post + "/RQ",       fmt::format("{} RICH RQ (post-cut);RQ;Events", label),            200, 0, 1);
        h.postcut.best_PID = svc->book_atomic_hist1D(post + "/best_PID", fmt::format("{} RICH best PID (post-cut);PID;Events", label),     800, -400, 400);
        h.postcut.best_ntot = svc->book_atomic_hist1D(post + "/best_ntot", fmt::format("{} RICH best ntot (post-cut);ntot;Events", label),     800, -400, 400);
    }

    void fill_precut_histograms(const clas12::Particle& particle, const clas12::RICHParticleBank& rich, HadronHistograms& h) {
        if (h.precut.p)        h.precut.p->fill(particle.p());
        if (h.precut.theta)    h.precut.theta->fill(particle.theta());
        if (h.precut.phi)      h.precut.phi->fill(particle.phi());
        if (h.precut.vz)       h.precut.vz->fill(particle.vz());
        if (h.precut.RQ)       h.precut.RQ->fill(rich.RQ);
        if (h.precut.best_PID) h.precut.best_PID->fill(rich.best_PID);
        if (h.precut.best_ntot) h.precut.best_ntot->fill(rich.best_ntot);
    }

    void fill_postcut_histograms(const clas12::Particle& particle, const clas12::RICHParticleBank& rich, HadronHistograms& h) {
        if (h.postcut.p)        h.postcut.p->fill(particle.p());
        if (h.postcut.theta)    h.postcut.theta->fill(particle.theta());
        if (h.postcut.phi)      h.postcut.phi->fill(particle.phi());
        if (h.postcut.vz)       h.postcut.vz->fill(particle.vz());
        if (h.postcut.RQ)       h.postcut.RQ->fill(rich.RQ);
        if (h.postcut.best_PID) h.postcut.best_PID->fill(rich.best_PID);
        if (h.postcut.best_ntot) h.postcut.best_ntot->fill(rich.best_ntot);
    }

    // --- Exclusive mode histograms ---
    struct ExclusiveHistograms {
        // ed -> ep pi+ pi- (n)
        clas12::framework::AtomicHistogram1D* mm_all = nullptr;
        clas12::framework::AtomicHistogram1D* mm_selected = nullptr;
        clas12::framework::AtomicHistogram1D* mm_squared = nullptr;
        clas12::framework::AtomicHistogram1D* n_combinations = nullptr;
        clas12::framework::AtomicHistogram1D* piplus_p = nullptr;
        clas12::framework::AtomicHistogram1D* piplus_theta = nullptr;
        clas12::framework::AtomicHistogram1D* piminus_p = nullptr;
        clas12::framework::AtomicHistogram1D* piminus_theta = nullptr;

        // ed -> ep K+ K- (n)
        clas12::framework::AtomicHistogram1D* kaon_mm_all = nullptr;
        clas12::framework::AtomicHistogram1D* kaon_mm_selected = nullptr;
        clas12::framework::AtomicHistogram1D* kaon_mm_squared = nullptr;
        clas12::framework::AtomicHistogram1D* kaon_n_combinations = nullptr;
        clas12::framework::AtomicHistogram1D* kplus_p = nullptr;
        clas12::framework::AtomicHistogram1D* kplus_theta = nullptr;
        clas12::framework::AtomicHistogram1D* kminus_p = nullptr;
        clas12::framework::AtomicHistogram1D* kminus_theta = nullptr;

        // Exclusivity variables per channel
        ExclusivityHistograms pion_excl;
        ExclusivityHistograms kaon_excl;

        // Hadron cut variables (all candidates, before quality cuts)
        ParticleCutHistograms proton_cuts;
        ParticleCutHistograms piplus_cuts;
        ParticleCutHistograms piminus_cuts;
        ParticleCutHistograms kplus_cuts;
        ParticleCutHistograms kminus_cuts;
    };

    ExclusiveHistograms h_exclusive;

    void book_exclusive_histograms() {
        auto* svc = m_hist;
        auto dir = std::string("/AiPidHadron/Exclusive");

        h_exclusive.mm_all = svc->book_atomic_hist1D(
            dir + "/missing_mass_all",
            "Missing Mass ed#rightarrowep#pi^{+}#pi^{-}(X) (all combos);MM [GeV/c^{2}];Events",
            300, 0.0, 2.0);

        h_exclusive.mm_selected = svc->book_atomic_hist1D(
            dir + "/missing_mass_selected",
            "Missing Mass ed#rightarrowep#pi^{+}#pi^{-}(X) (selected);MM [GeV/c^{2}];Events",
            300, 0.0, 2.0);

        h_exclusive.mm_squared = svc->book_atomic_hist1D(
            dir + "/missing_mass_squared",
            "Missing Mass^{2} ed#rightarrowep#pi^{+}#pi^{-}(X);MM^{2} [GeV^{2}/c^{4}];Events",
            300, -1.0, 3.0);

        h_exclusive.n_combinations = svc->book_atomic_hist1D(
            dir + "/n_combinations",
            "Number of (p,#pi^{+},#pi^{-}) combinations per event;N_{combos};Events",
            20, 0, 20);

        h_exclusive.piplus_p = svc->book_atomic_hist1D(
            dir + "/piplus_momentum",
            "#pi^{+} Momentum (exclusive);p [GeV/c];Events",
            200, 0, 3);

        h_exclusive.piplus_theta = svc->book_atomic_hist1D(
            dir + "/piplus_theta",
            "#pi^{+} #theta (exclusive);#theta [deg];Events",
            200, 0, 180);

        h_exclusive.piminus_p = svc->book_atomic_hist1D(
            dir + "/piminus_momentum",
            "#pi^{-} Momentum (exclusive);p [GeV/c];Events",
            200, 0, 3);

        h_exclusive.piminus_theta = svc->book_atomic_hist1D(
            dir + "/piminus_theta",
            "#pi^{-} #theta (exclusive);#theta [deg];Events",
            200, 0, 180);

        // --- ed -> ep K+ K- (n) ---
        auto kdir = std::string("/AiPidHadron/ExclusiveKaon");

        h_exclusive.kaon_mm_all = svc->book_atomic_hist1D(
            kdir + "/missing_mass_all",
            "Missing Mass ed#rightarrowepK^{+}K^{-}(X) (all combos);MM [GeV/c^{2}];Events",
            300, 0.0, 2.0);

        h_exclusive.kaon_mm_selected = svc->book_atomic_hist1D(
            kdir + "/missing_mass_selected",
            "Missing Mass ed#rightarrowepK^{+}K^{-}(X) (selected);MM [GeV/c^{2}];Events",
            300, 0.0, 2.0);

        h_exclusive.kaon_mm_squared = svc->book_atomic_hist1D(
            kdir + "/missing_mass_squared",
            "Missing Mass^{2} ed#rightarrowepK^{+}K^{-}(X);MM^{2} [GeV^{2}/c^{4}];Events",
            300, -1.0, 3.0);

        h_exclusive.kaon_n_combinations = svc->book_atomic_hist1D(
            kdir + "/n_combinations",
            "Number of (p,K^{+},K^{-}) combinations per event;N_{combos};Events",
            20, 0, 20);

        h_exclusive.kplus_p = svc->book_atomic_hist1D(
            kdir + "/kplus_momentum",
            "K^{+} Momentum (exclusive);p [GeV/c];Events",
            200, 0, 3);

        h_exclusive.kplus_theta = svc->book_atomic_hist1D(
            kdir + "/kplus_theta",
            "K^{+} #theta (exclusive);#theta [deg];Events",
            200, 0, 180);

        h_exclusive.kminus_p = svc->book_atomic_hist1D(
            kdir + "/kminus_momentum",
            "K^{-} Momentum (exclusive);p [GeV/c];Events",
            200, 0, 3);

        h_exclusive.kminus_theta = svc->book_atomic_hist1D(
            kdir + "/kminus_theta",
            "K^{-} #theta (exclusive);#theta [deg];Events",
            200, 0, 180);

        // --- Exclusivity histograms ---
        auto book_excl_hists = [&](ExclusivityHistograms& eh, const std::string& base, const std::string& label) {
            eh.missing_energy = svc->book_atomic_hist1D(base + "/missing_energy",
                fmt::format("{} Missing Energy;#DeltaE [GeV];Events", label), 300, -2.0, 4.0);
            eh.missing_p = svc->book_atomic_hist1D(base + "/missing_momentum",
                fmt::format("{} Missing Momentum;|p_{{miss}}| [GeV/c];Events", label), 300, 0.0, 4.0);
            eh.missing_pt = svc->book_atomic_hist1D(base + "/missing_pt",
                fmt::format("{} Missing p_{{T}};p_{{T,miss}} [GeV/c];Events", label), 200, 0.0, 2.0);
            eh.missing_theta = svc->book_atomic_hist1D(base + "/missing_theta",
                fmt::format("{} Missing #theta;#theta_{{miss}} [deg];Events", label), 200, 0, 180);
            eh.missing_phi = svc->book_atomic_hist1D(base + "/missing_phi",
                fmt::format("{} Missing #phi;#phi_{{miss}} [deg];Events", label), 200, -180, 180);
            // Neutron detection histograms
            auto ndir = base + "/Neutron";
            eh.n_detected = svc->book_atomic_hist1D(ndir + "/n_detected",
                fmt::format("{} Number of detected neutrons;N_{{n}};Events", label), 10, 0, 10);
            eh.neutron_opening_angle = svc->book_atomic_hist1D(ndir + "/opening_angle",
                fmt::format("{} Neutron-Missing opening angle;#Delta#alpha [deg];Events", label), 200, 0, 180);
            eh.neutron_p = svc->book_atomic_hist1D(ndir + "/neutron_p",
                fmt::format("{} Detected neutron momentum;p_{{n}} [GeV/c];Events", label), 200, 0, 4);
            eh.neutron_beta = svc->book_atomic_hist1D(ndir + "/neutron_beta",
                fmt::format("{} Detected neutron #beta;#beta_{{n}};Events", label), 200, 0, 1.2);
            eh.mm_with_neutron = svc->book_atomic_hist1D(ndir + "/mm_with_neutron",
                fmt::format("{} MM (with detected n);MM [GeV/c^{{2}}];Events", label), 300, -1.0, 2.0);
        };

        book_excl_hists(h_exclusive.pion_excl, dir, "ed#rightarrowep#pi^{+}#pi^{-}(X)");
        book_excl_hists(h_exclusive.kaon_excl, kdir, "ed#rightarrowepK^{+}K^{-}(X)");

        // --- Hadron cut variable histograms ---
        auto book_cut_hists = [&](ParticleCutHistograms& h, const std::string& base, const std::string& label) {
            h.chi2pid  = svc->book_atomic_hist1D(base + "/chi2pid",  fmt::format("{} #chi^{{2}}_{{PID}};#chi^{{2}}_{{PID}};Events", label), 200, -10, 10);
            h.p        = svc->book_atomic_hist1D(base + "/p",        fmt::format("{} Momentum;p [GeV/c];Events", label),                    200, 0, 10);
            h.vz       = svc->book_atomic_hist1D(base + "/vz",       fmt::format("{} v_{{z}};v_{{z}} [cm];Events", label),                  200, -30, 15);
            h.delta_vz = svc->book_atomic_hist1D(base + "/delta_vz", fmt::format("{} #Deltav_{{z}};#Deltav_{{z}} [cm];Events", label),      200, -10, 10);
        };

        auto cuts_dir = std::string("/AiPidHadron/HadronCuts");
        book_cut_hists(h_exclusive.proton_cuts,  cuts_dir + "/proton",  "Proton");
        book_cut_hists(h_exclusive.piplus_cuts,   cuts_dir + "/piplus",  "#pi^{+}");
        book_cut_hists(h_exclusive.piminus_cuts,  cuts_dir + "/piminus", "#pi^{-}");
        book_cut_hists(h_exclusive.kplus_cuts,    cuts_dir + "/kplus",   "K^{+}");
        book_cut_hists(h_exclusive.kminus_cuts,   cuts_dir + "/kminus",  "K^{-}");
    }
};

}  // namespace ai_pid_hadron
