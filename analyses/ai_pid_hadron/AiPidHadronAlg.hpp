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

#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
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

    static constexpr auto ntuple_fields() {
        using T = HadronTreeData;
        return std::make_tuple(
            // Particle
            framework::field("rich_pid", &T::rich_pid),
            framework::field("pid", &T::pid),
            framework::field("truth_pid", &T::truth_pid),
            framework::field("truth_source", &T::truth_source),
            framework::field("missing_mass", &T::missing_mass),
            framework::field("missing_energy", &T::missing_energy),
            framework::field("missing_px", &T::missing_px),
            framework::field("missing_py", &T::missing_py),
            framework::field("missing_pz", &T::missing_pz),
            framework::field("missing_p", &T::missing_p),
            framework::field("missing_pt", &T::missing_pt),
            framework::field("missing_theta", &T::missing_theta),
            framework::field("missing_phi", &T::missing_phi),
            framework::field("p", &T::p),
            framework::field("theta", &T::theta),
            framework::field("phi", &T::phi),
            framework::field("beta", &T::beta),
            framework::field("charge", &T::charge),
            // PCAL
            framework::field("PCAL_E", &T::PCAL_E),
            framework::field("PCAL_chi2", &T::PCAL_chi2),
            framework::field("PCAL_time", &T::PCAL_time),
            framework::field("PCAL_path", &T::PCAL_path),
            framework::field("PCAL_lu", &T::PCAL_lu),
            framework::field("PCAL_lv", &T::PCAL_lv),
            framework::field("PCAL_lw", &T::PCAL_lw),
            framework::field("PCAL_du", &T::PCAL_du),
            framework::field("PCAL_dv", &T::PCAL_dv),
            framework::field("PCAL_dw", &T::PCAL_dw),
            framework::field("PCAL_m2u", &T::PCAL_m2u),
            framework::field("PCAL_m2v", &T::PCAL_m2v),
            framework::field("PCAL_m2w", &T::PCAL_m2w),
            framework::field("PCAL_m3u", &T::PCAL_m3u),
            framework::field("PCAL_m3v", &T::PCAL_m3v),
            framework::field("PCAL_m3w", &T::PCAL_m3w),
            framework::field("PCAL_dbstU", &T::PCAL_dbstU),
            framework::field("PCAL_dbstV", &T::PCAL_dbstV),
            framework::field("PCAL_dbstW", &T::PCAL_dbstW),
            framework::field("PCAL_rawEU", &T::PCAL_rawEU),
            framework::field("PCAL_rawEV", &T::PCAL_rawEV),
            framework::field("PCAL_rawEW", &T::PCAL_rawEW),
            framework::field("PCAL_recEU", &T::PCAL_recEU),
            framework::field("PCAL_recEV", &T::PCAL_recEV),
            framework::field("PCAL_recEW", &T::PCAL_recEW),
            framework::field("PCAL_recDTU", &T::PCAL_recDTU),
            framework::field("PCAL_recDTV", &T::PCAL_recDTV),
            framework::field("PCAL_recDTW", &T::PCAL_recDTW),
            framework::field("PCAL_recFTU", &T::PCAL_recFTU),
            framework::field("PCAL_recFTV", &T::PCAL_recFTV),
            framework::field("PCAL_recFTW", &T::PCAL_recFTW),
            framework::field("PCAL_size", &T::PCAL_size),
            // ECIN
            framework::field("ICAL_E", &T::ICAL_E),
            framework::field("ICAL_chi2", &T::ICAL_chi2),
            framework::field("ICAL_time", &T::ICAL_time),
            framework::field("ICAL_path", &T::ICAL_path),
            framework::field("ICAL_lu", &T::ICAL_lu),
            framework::field("ICAL_lv", &T::ICAL_lv),
            framework::field("ICAL_lw", &T::ICAL_lw),
            framework::field("ICAL_du", &T::ICAL_du),
            framework::field("ICAL_dv", &T::ICAL_dv),
            framework::field("ICAL_dw", &T::ICAL_dw),
            framework::field("ICAL_m2u", &T::ICAL_m2u),
            framework::field("ICAL_m2v", &T::ICAL_m2v),
            framework::field("ICAL_m2w", &T::ICAL_m2w),
            framework::field("ICAL_m3u", &T::ICAL_m3u),
            framework::field("ICAL_m3v", &T::ICAL_m3v),
            framework::field("ICAL_m3w", &T::ICAL_m3w),
            framework::field("ICAL_dbstU", &T::ICAL_dbstU),
            framework::field("ICAL_dbstV", &T::ICAL_dbstV),
            framework::field("ICAL_dbstW", &T::ICAL_dbstW),
            framework::field("ICAL_rawEU", &T::ICAL_rawEU),
            framework::field("ICAL_rawEV", &T::ICAL_rawEV),
            framework::field("ICAL_rawEW", &T::ICAL_rawEW),
            framework::field("ICAL_recEU", &T::ICAL_recEU),
            framework::field("ICAL_recEV", &T::ICAL_recEV),
            framework::field("ICAL_recEW", &T::ICAL_recEW),
            framework::field("ICAL_recDTU", &T::ICAL_recDTU),
            framework::field("ICAL_recDTV", &T::ICAL_recDTV),
            framework::field("ICAL_recDTW", &T::ICAL_recDTW),
            framework::field("ICAL_recFTU", &T::ICAL_recFTU),
            framework::field("ICAL_recFTV", &T::ICAL_recFTV),
            framework::field("ICAL_recFTW", &T::ICAL_recFTW),
            framework::field("ICAL_size", &T::ICAL_size),
            // ECOUT
            framework::field("OCAL_E", &T::OCAL_E),
            framework::field("OCAL_chi2", &T::OCAL_chi2),
            framework::field("OCAL_time", &T::OCAL_time),
            framework::field("OCAL_path", &T::OCAL_path),
            framework::field("OCAL_lu", &T::OCAL_lu),
            framework::field("OCAL_lv", &T::OCAL_lv),
            framework::field("OCAL_lw", &T::OCAL_lw),
            framework::field("OCAL_du", &T::OCAL_du),
            framework::field("OCAL_dv", &T::OCAL_dv),
            framework::field("OCAL_dw", &T::OCAL_dw),
            framework::field("OCAL_m2u", &T::OCAL_m2u),
            framework::field("OCAL_m2v", &T::OCAL_m2v),
            framework::field("OCAL_m2w", &T::OCAL_m2w),
            framework::field("OCAL_m3u", &T::OCAL_m3u),
            framework::field("OCAL_m3v", &T::OCAL_m3v),
            framework::field("OCAL_m3w", &T::OCAL_m3w),
            framework::field("OCAL_dbstU", &T::OCAL_dbstU),
            framework::field("OCAL_dbstV", &T::OCAL_dbstV),
            framework::field("OCAL_dbstW", &T::OCAL_dbstW),
            framework::field("OCAL_rawEU", &T::OCAL_rawEU),
            framework::field("OCAL_rawEV", &T::OCAL_rawEV),
            framework::field("OCAL_rawEW", &T::OCAL_rawEW),
            framework::field("OCAL_recEU", &T::OCAL_recEU),
            framework::field("OCAL_recEV", &T::OCAL_recEV),
            framework::field("OCAL_recEW", &T::OCAL_recEW),
            framework::field("OCAL_recDTU", &T::OCAL_recDTU),
            framework::field("OCAL_recDTV", &T::OCAL_recDTV),
            framework::field("OCAL_recDTW", &T::OCAL_recDTW),
            framework::field("OCAL_recFTU", &T::OCAL_recFTU),
            framework::field("OCAL_recFTV", &T::OCAL_recFTV),
            framework::field("OCAL_recFTW", &T::OCAL_recFTW),
            framework::field("OCAL_size", &T::OCAL_size),
            // HTCC
            framework::field("HTCC_nphe", &T::HTCC_nphe),
            framework::field("HTCC_time", &T::HTCC_time),
            framework::field("HTCC_path", &T::HTCC_path),
            framework::field("HTCC_chi2", &T::HTCC_chi2),
            framework::field("HTCC_dtheta", &T::HTCC_dtheta),
            framework::field("HTCC_dphi", &T::HTCC_dphi),
            // FTOF
            framework::field("FTOF_E", &T::FTOF_E),
            framework::field("FTOF_time", &T::FTOF_time),
            framework::field("FTOF_path", &T::FTOF_path),
            framework::field("FTOF_chi2", &T::FTOF_chi2),
            framework::field("FTOF_dedx", &T::FTOF_dedx),
            framework::field("FTOF_size", &T::FTOF_size),
            framework::field("FTOF_layermult", &T::FTOF_layermult)
        );
    }
};

/**
 * @brief Algorithm for AI PID hadron analysis
 *
 * Supports two truth PID modes:
 * - "rich": Uses RICH::Particle best_PID as truth label
 * - "exclusive": Uses ed -> ep(n)pi+pi- reaction topology as truth label
 */
/// Configuration for AiPidHadronAlg.
struct AiPidHadronConfig {
    std::string truth_mode = "rich";
};

class AiPidHadronAlg : public framework::Algorithm {
public:
    using Config = AiPidHadronConfig;
    using StatusCode = framework::StatusCode;

    explicit AiPidHadronAlg(std::string name = "AiPidHadronAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config) {
        if (!m_config.truth_mode.empty()) m_truth_mode.set(m_config.truth_mode);
        required_services(m_hist, m_ntuple_svc);
        declare_banks("REC::Particle", "RICH::Particle", "REC::Calorimeter", "REC::CaloExtras", "REC::Cherenkov", "REC::Scintillator", "REC::ScintExtras");
        declare_properties(m_enable_histograms, m_enable_ntuple, m_truth_mode, m_beam_energy, m_mm_center, m_mm_width);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        framework::Log::info(name(), "Truth PID mode: {}", m_truth_mode.get());

        if (m_truth_mode.get() == "rich") {
            if (m_enable_histograms.get()) {
                book_histograms("/AiPidHadron/PiPlus",  h_piplus,  "#pi^{+}");
                book_histograms("/AiPidHadron/PiMinus", h_piminus, "#pi^{-}");
                book_histograms("/AiPidHadron/KPlus",   h_kplus,   "K^{+}");
                book_histograms("/AiPidHadron/KMinus",  h_kminus,  "K^{-}");

                // RICH vs REC PID comparison
                book_comparison_histograms();
            }

            auto init_cutflow = [](framework::Cutflow& cf) {
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
            framework::Log::info(name(), "Exclusive mode: ed -> ep(n)pi+pi-");
            framework::Log::info(name(), "  Beam energy: {} GeV", m_beam_energy.get());
            framework::Log::info(name(), "  MM cut: {} +/- {} GeV", m_mm_center.get(), m_mm_width.get());

            if (m_enable_histograms.get()) {
                book_exclusive_histograms();
            }

            m_cutflow_exclusive.add_cut("Total events");
            m_cutflow_exclusive.add_cut("Has good electron");
            m_cutflow_exclusive.add_cut("Passes DIS cuts");
            m_cutflow_exclusive.add_cut("Has >= 1 proton");
            m_cutflow_exclusive.add_cut("Has meson pair");
            m_cutflow_exclusive.add_cut("Missing mass cut");
        } else {
            framework::Log::error(name(), "Unknown truth_mode '{}'. Use 'rich' or 'exclusive'.", m_truth_mode.get());
            return framework::StatusCode::FAILURE();
        }

        if (m_enable_ntuple.get()) {
            m_ntuple.init(m_ntuple_svc.get(), "rich_hadrons");
        }

        return framework::StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> framework::StatusCode override {
        if (m_truth_mode.get() == "exclusive") {
            return process_exclusive(event);
        }
        return process_rich(event);
    }

    auto on_finalize() -> framework::StatusCode override {
        if (m_ntuple) {
            framework::Log::info(name(), "N-tuple entries: {}", m_ntuple.service()->get_entries(m_ntuple.tree_name()));
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
                    auto pid_enum = to_pid_enum(pid);
                    std::string pname = pid_enum ? pid_name(*pid_enum) : fmt::format("Unknown({})", pid);
                    double frac = total > 0 ? 100.0 * static_cast<double>(count) / static_cast<double>(total) : 0.0;
                    rows.push_back({fmt::format("{}", pid), pname, fmt::format("{}", count), fmt::format("{:.2f}%", frac)});
                }
                framework::Log::print_yield_summary_table(
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
                framework::Log::print_yield_summary_table(
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
                    auto pid_enum = to_pid_enum(pid);
                    std::string pname = pid_enum ? pid_name(*pid_enum) : fmt::format("Unknown({})", pid);
                    double frac = total > 0 ? 100.0 * static_cast<double>(count) / static_cast<double>(total) : 0.0;
                    rows.push_back({fmt::format("{}", pid), pname, fmt::format("{}", count), fmt::format("{:.2f}%", frac)});
                }
                framework::Log::print_yield_summary_table(
                    "Exclusive PID Summary",
                    {"PID", "Name", "Count", "Fraction"},
                    rows, fmt::format("Total: {}", total));
            }
        }

        return framework::StatusCode::SUCCESS();
    }

private:
    // --- RICH vs REC PID comparison structs ---
    struct KinematicHistograms {
        framework::H1<> p{mh::regular(0, 10, static_cast<std::size_t>(200))};
        framework::H1<> theta{mh::regular(0, 40, static_cast<std::size_t>(200))};
        framework::H1<> phi{mh::regular(-180, 180, static_cast<std::size_t>(200))};
        framework::H1<> beta{mh::regular(0, 1.2, static_cast<std::size_t>(200))};
    };

    struct PidCompareHistograms {
        KinematicHistograms agree;
        KinematicHistograms disagree;
        framework::H1<> rich_pid_dist{mh::regular(-400, 400, static_cast<std::size_t>(800))}; // RICH PID distribution for this REC species
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
        framework::H1<> chi2pid{mh::regular(-10, 10, static_cast<std::size_t>(200))};
        framework::H1<> p{mh::regular(0, 10, static_cast<std::size_t>(200))};
        framework::H1<> vz{mh::regular(-30, 30, static_cast<std::size_t>(200))};
        framework::H1<> delta_vz{mh::regular(-10, 10, static_cast<std::size_t>(200))};
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
        framework::H1<> missing_energy{mh::regular(-2.0, 4.0, static_cast<std::size_t>(300))};
        framework::H1<> missing_p{mh::regular(0.0, 4.0, static_cast<std::size_t>(300))};
        framework::H1<> missing_pt{mh::regular(0.0, 2.0, static_cast<std::size_t>(200))};
        framework::H1<> missing_theta{mh::regular(0, 180, static_cast<std::size_t>(200))};
        framework::H1<> missing_phi{mh::regular(-180, 180, static_cast<std::size_t>(200))};
        // Detected neutron (filled only when neutron found)
        framework::H1<> n_detected{mh::regular(0, 10, static_cast<std::size_t>(10))};
        framework::H1<> neutron_opening_angle{mh::regular(0, 180, static_cast<std::size_t>(200))};
        framework::H1<> neutron_p{mh::regular(0, 4, static_cast<std::size_t>(200))};
        framework::H1<> neutron_beta{mh::regular(0, 1.2, static_cast<std::size_t>(200))};
        framework::H1<> mm_with_neutron{mh::regular(-1.0, 2.0, static_cast<std::size_t>(300))}; // MM(ed→epn+mesons)
    };

    // --- Properties ---
    framework::Property<bool> m_enable_histograms{
        "enable_histograms", true, "Enable histogram filling"
    };
    framework::Property<std::string> m_truth_mode{
        "truth_mode", "rich", "Truth PID mode: 'rich' or 'exclusive'"
    };
    framework::Property<double> m_beam_energy{
        "beam_energy", 10.53, "Beam energy [GeV]"
    };
    framework::Property<double> m_mm_center{
        "mm_center", 0.9396, "Missing mass cut center [GeV] (neutron mass)"
    };
    framework::Property<double> m_mm_width{
        "mm_width", 0.2, "Missing mass cut half-width [GeV]"
    };
    framework::Property<bool> m_enable_ntuple{
        "enable_ntuple", false, "Enable N-tuple output"
    };

    // --- Services ---
    Config m_config;

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::NTupleService> m_ntuple_svc;

    // N-tuple writer
    framework::NTupleWriter<HadronTreeData> m_ntuple;

    // --- RICH mode members ---
    framework::Cutflow m_cutflow_piplus{"π⁺ RICH Selection"};
    framework::Cutflow m_cutflow_piminus{"π⁻ RICH Selection"};
    framework::Cutflow m_cutflow_kplus{"K⁺ RICH Selection"};
    framework::Cutflow m_cutflow_kminus{"K⁻ RICH Selection"};
    std::mutex m_pid_count_mutex;
    std::map<int, size_t> m_rich_pid_counts;
    std::map<int, PidAgreement> m_pid_agreement;

    // RICH vs REC PID comparison histograms (Sector 1 only)
    framework::H2<> h_rich_vs_rec_pid{mh::regular(0, 5, static_cast<std::size_t>(5)), mh::regular(0, 5, static_cast<std::size_t>(5))};
    PidCompareHistograms h_cmp_piplus;
    PidCompareHistograms h_cmp_piminus;
    PidCompareHistograms h_cmp_kplus;
    PidCompareHistograms h_cmp_kminus;

    // Per-RICH-species REC PID distribution (Sector 1)
    framework::H1<> h_rec_pid_for_rich_piplus{mh::regular(-400, 400, static_cast<std::size_t>(800))};
    framework::H1<> h_rec_pid_for_rich_piminus{mh::regular(-400, 400, static_cast<std::size_t>(800))};
    framework::H1<> h_rec_pid_for_rich_kplus{mh::regular(-400, 400, static_cast<std::size_t>(800))};
    framework::H1<> h_rec_pid_for_rich_kminus{mh::regular(-400, 400, static_cast<std::size_t>(800))};

    // --- Exclusive mode members ---
    framework::Cutflow m_cutflow_exclusive{"Exclusive ed→ep(n)π⁺π⁻"};
    std::mutex m_excl_count_mutex;
    std::map<int, size_t> m_excl_pid_counts;

    // ========================================================================
    // RICH mode processing
    // ========================================================================

    auto process_rich(framework::Event& event) -> framework::StatusCode {
        // Detector banks below are read via Particle's accessors, which
        // route to the per-event generated-wrapper cache.

        if (event.get_bank("RICH::Particle").rows() == 0) {
            return framework::StatusCode::FILTERED();
        }

        // Loop over all charged hadron candidates and look up their RICH info
        static constexpr std::array<PIDs, 4> hadron_pids = {
            PIDs::PIPLUS, PIDs::PIMINUS,
            PIDs::KPLUS, PIDs::KMINUS};
        for (auto pid_type : hadron_pids) {
            for (const auto& particle : event.particles(pid_type)) {
                // Determine cutflow based on REC PID
                framework::Cutflow* cf = nullptr;
                if (pid_type == PIDs::PIPLUS)       cf = &m_cutflow_piplus;
                else if (pid_type == PIDs::PIMINUS) cf = &m_cutflow_piminus;
                else if (pid_type == PIDs::KPLUS)   cf = &m_cutflow_kplus;
                else if (pid_type == PIDs::KMINUS)  cf = &m_cutflow_kminus;

                cf->pass("RICH hadrons");
                auto rich = read_RICHParticle_bank(event, particle.index());

                // Determine histogram set based on RICH best_PID
                int best_pid = rich.best_PID;
                HadronHistograms* h = nullptr;

                if (best_pid == static_cast<int>(PIDs::PIPLUS)) {
                    h = &h_piplus;
                } else if (best_pid == static_cast<int>(PIDs::PIMINUS)) {
                    h = &h_piminus;
                } else if (best_pid == static_cast<int>(PIDs::KPLUS)) {
                    h = &h_kplus;
                } else if (best_pid == static_cast<int>(PIDs::KMINUS)) {
                    h = &h_kminus;
                }

                // Count all RICH best_PIDs (before any cuts)
                const int rec_pid = static_cast<int>(pid_type);
                {
                    std::lock_guard<std::mutex> lock(m_pid_count_mutex);
                    m_rich_pid_counts[best_pid]++;
                }

                // Determine particle sector from scintillator
                auto ftof = particle.scintillator();
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
                cf->pass("RICH PID match");
                // Fill pre-cut histograms
                fill_precut_histograms(particle, rich, *h);

                // Apply quality cuts
                if (rich.RQ <= 0.1f) continue;
                cf->pass("RQ > 0.1");
                if (rich.best_ntot <= 2.5f) continue;
                cf->pass("best_ntot > 2.5");
                // Fill post-cut histograms
                fill_postcut_histograms(particle, rich, *h);

                // Fill TTree
                if (m_ntuple) {
                    auto calo = particle.calo();
                    auto htcc = particle.cherenkov();
                    auto ftof_nt = particle.scintillator();

                    populate_particle_data(particle, rich);
                    populate_calorimeter_data(calo);
                    populate_cherenkov_data(htcc);
                    populate_scintillator_data(ftof_nt);
                    m_ntuple.fill();
                }
            }
        }

        return framework::StatusCode::SUCCESS();
    }

    // ========================================================================
    // Exclusive mode processing: ed -> ep(n)pi+pi-
    // ========================================================================

    auto process_exclusive(framework::Event& event) -> framework::StatusCode {
        m_cutflow_exclusive.pass("Total events");

        // 1. Require good DIS electron from upstream
        auto electron_opt = framework::EventHelpers::get_dis_electron(event);
        if (!electron_opt) return framework::StatusCode::FILTERED();
        const auto& electron = *electron_opt;
        m_cutflow_exclusive.pass("Has good electron");
        m_cutflow_exclusive.pass("Passes DIS cuts");

        // 2. Read hadrons via Event's per-PID accessors. Detector banks
        // for selected particles are read on demand through the per-event
        // generated-wrapper cache inside fill_exclusive_ntuple.
        const auto& protons     = event.protons();
        const auto& pions_plus  = event.pions_plus();
        const auto& pions_minus = event.pions_minus();
        const auto& kaons_plus  = event.particles(PIDs::KPLUS);
        const auto& kaons_minus = event.particles(PIDs::KMINUS);
        const auto& neutrons    = event.particles(PIDs::NEUTRON);

        // 3. Topology check: require proton + (pi+pi- or K+K-)
        if (protons.empty()) return framework::StatusCode::FILTERED();
        m_cutflow_exclusive.pass("Has >= 1 proton");

        bool has_pion_pair = !pions_plus.empty() && !pions_minus.empty();
        bool has_kaon_pair = !kaons_plus.empty() && !kaons_minus.empty();

        if (!has_pion_pair && !has_kaon_pair) return framework::StatusCode::FILTERED();
        m_cutflow_exclusive.pass("Has meson pair");

        // 3b. Fill hadron cut variable histograms (before quality cuts)
        const double e_vz = electron.vz();
        if (m_enable_histograms) {
            fill_all_cut_histograms(e_vz, protons, pions_plus, pions_minus, kaons_plus, kaons_minus);
        }

        // 4. Construct beam and deuterium target
        const double beam_E = m_beam_energy.get();
        Particle beam(11, 0, -1, -1, Constants::ELECTRONMASS,
                              0, 0, beam_E, beam_E, 0, 0, 0, 0, 1.0, 0);

        Particle target(static_cast<int>(PIDs::DEUTERON), 0, -1, 0,
                                Constants::DEUTERIUMMASS,
                                0, 0, 0, Constants::DEUTERIUMMASS, 0, 0, 0, 0, 0, 0);

        // 5. Loop over all (p, pi+, pi-) combinations
        const int n_combos = static_cast<int>(protons.size() * pions_plus.size() * pions_minus.size());
        h_exclusive.n_combinations.fill(n_combos);

        bool any_passed_mm = false;

        for (const auto& proton : protons) {
            if (!passes_hadron_cuts(proton, e_vz)) continue;
            for (const auto& pip : pions_plus) {
                if (!passes_hadron_cuts(pip, e_vz)) continue;
                for (const auto& pim : pions_minus) {
                    if (!passes_hadron_cuts(pim, e_vz)) continue;

                    // Detected final state: e, p, pi+, pi-
                    std::vector<Particle> detected = {electron, proton, pip, pim};
                    auto excl = compute_exclusivity(beam, target, detected);

                    // Fill pre-cut missing mass histograms (without neutron)
                    h_exclusive.mm_all.fill(excl.missing_mass);
                    h_exclusive.mm_squared.fill(excl.missing_mass2);
                    // Neutron detection
                    h_exclusive.pion_excl.n_detected.fill(static_cast<int>(neutrons.size()));
                    if (!neutrons.empty()) {
                        // Find neutron closest to missing momentum direction
                        ROOT::Math::XYZVector miss_dir(excl.missing_px, excl.missing_py, excl.missing_pz);
                        double best_angle = 999;
                        const Particle* best_n = nullptr;
                        for (const auto& n : neutrons) {
                            double angle = ROOT::Math::VectorUtil::Angle(miss_dir, n.PxPyPzVector()) * 180.0 / M_PI;
                            if (angle < best_angle) { best_angle = angle; best_n = &n; }
                        }
                        if (best_n) {
                            h_exclusive.pion_excl.neutron_opening_angle.fill(best_angle);
                            h_exclusive.pion_excl.neutron_p.fill(best_n->p());
                            h_exclusive.pion_excl.neutron_beta.fill(best_n->beta());
                            // Exclusivity variables with neutron: ed → e p n π+ π- (X)
                            std::vector<Particle> detected_with_n = {electron, proton, pip, pim, *best_n};
                            auto excl_n = compute_exclusivity(beam, target, detected_with_n);
                            h_exclusive.pion_excl.mm_with_neutron.fill(excl_n.missing_mass);
                            h_exclusive.pion_excl.missing_energy.fill(excl_n.missing_energy);
                            h_exclusive.pion_excl.missing_p.fill(excl_n.missing_p);
                            h_exclusive.pion_excl.missing_pt.fill(excl_n.missing_pt);
                            h_exclusive.pion_excl.missing_theta.fill(excl_n.missing_theta);
                            h_exclusive.pion_excl.missing_phi.fill(excl_n.missing_phi);
                        }
                    }

                    // Apply missing mass cut: |MM - neutron_mass| < width
                    if (std::abs(excl.missing_mass - m_mm_center.get()) > m_mm_width.get()) continue;

                    any_passed_mm = true;

                    // Fill post-cut histograms
                    h_exclusive.mm_selected.fill(excl.missing_mass);
                    h_exclusive.piplus_p.fill(pip.p());
                    h_exclusive.piplus_theta.fill(pip.theta());
                    h_exclusive.piminus_p.fill(pim.p());
                    h_exclusive.piminus_theta.fill(pim.theta());
                    // Fill NTuple for pi+ (true PID = 211) and pi- (true PID = -211)
                    if (m_ntuple) {
                        fill_exclusive_ntuple(pip, static_cast<int>(PIDs::PIPLUS), excl);
                        fill_exclusive_ntuple(pim, static_cast<int>(PIDs::PIMINUS), excl);
                    }

                    // Count exclusive PIDs
                    {
                        std::lock_guard<std::mutex> lock(m_excl_count_mutex);
                        m_excl_pid_counts[static_cast<int>(PIDs::PIPLUS)]++;
                        m_excl_pid_counts[static_cast<int>(PIDs::PIMINUS)]++;
                    }
                }
            }
        }

        // 6. Loop over all (p, K+, K-) combinations
        const int kaon_n_combos = static_cast<int>(protons.size() * kaons_plus.size() * kaons_minus.size());
        h_exclusive.kaon_n_combinations.fill(kaon_n_combos);

        for (const auto& proton : protons) {
            if (!passes_hadron_cuts(proton, e_vz)) continue;
            for (const auto& kp : kaons_plus) {
                if (!passes_hadron_cuts(kp, e_vz)) continue;
                for (const auto& km : kaons_minus) {
                    if (!passes_hadron_cuts(km, e_vz)) continue;

                    std::vector<Particle> detected = {electron, proton, kp, km};
                    auto excl = compute_exclusivity(beam, target, detected);

                    // Fill pre-cut missing mass histograms (without neutron)
                    h_exclusive.kaon_mm_all.fill(excl.missing_mass);
                    h_exclusive.kaon_mm_squared.fill(excl.missing_mass2);
                    // Neutron detection
                    h_exclusive.kaon_excl.n_detected.fill(static_cast<int>(neutrons.size()));
                    if (!neutrons.empty()) {
                        ROOT::Math::XYZVector miss_dir(excl.missing_px, excl.missing_py, excl.missing_pz);
                        double best_angle = 999;
                        const Particle* best_n = nullptr;
                        for (const auto& n : neutrons) {
                            double angle = ROOT::Math::VectorUtil::Angle(miss_dir, n.PxPyPzVector()) * 180.0 / M_PI;
                            if (angle < best_angle) { best_angle = angle; best_n = &n; }
                        }
                        if (best_n) {
                            h_exclusive.kaon_excl.neutron_opening_angle.fill(best_angle);
                            h_exclusive.kaon_excl.neutron_p.fill(best_n->p());
                            h_exclusive.kaon_excl.neutron_beta.fill(best_n->beta());
                            // Exclusivity variables with neutron: ed → e p n K+ K- (X)
                            std::vector<Particle> detected_with_n = {electron, proton, kp, km, *best_n};
                            auto excl_n = compute_exclusivity(beam, target, detected_with_n);
                            h_exclusive.kaon_excl.mm_with_neutron.fill(excl_n.missing_mass);
                            h_exclusive.kaon_excl.missing_energy.fill(excl_n.missing_energy);
                            h_exclusive.kaon_excl.missing_p.fill(excl_n.missing_p);
                            h_exclusive.kaon_excl.missing_pt.fill(excl_n.missing_pt);
                            h_exclusive.kaon_excl.missing_theta.fill(excl_n.missing_theta);
                            h_exclusive.kaon_excl.missing_phi.fill(excl_n.missing_phi);
                        }
                    }

                    if (std::abs(excl.missing_mass - m_mm_center.get()) > m_mm_width.get()) continue;

                    any_passed_mm = true;

                    h_exclusive.kaon_mm_selected.fill(excl.missing_mass);
                    h_exclusive.kplus_p.fill(kp.p());
                    h_exclusive.kplus_theta.fill(kp.theta());
                    h_exclusive.kminus_p.fill(km.p());
                    h_exclusive.kminus_theta.fill(km.theta());
                    if (m_ntuple) {
                        fill_exclusive_ntuple(kp, static_cast<int>(PIDs::KPLUS), excl);
                        fill_exclusive_ntuple(km, static_cast<int>(PIDs::KMINUS), excl);
                    }

                    {
                        std::lock_guard<std::mutex> lock(m_excl_count_mutex);
                        m_excl_pid_counts[static_cast<int>(PIDs::KPLUS)]++;
                        m_excl_pid_counts[static_cast<int>(PIDs::KMINUS)]++;
                    }
                }
            }
        }

        if (any_passed_mm) {
            m_cutflow_exclusive.pass("Missing mass cut");
        }

        return any_passed_mm ? framework::StatusCode::SUCCESS()
                             : framework::StatusCode::FILTERED();
    }

    // ========================================================================
    // Hadron quality cuts
    // ========================================================================

    void fill_cut_histograms(ParticleCutHistograms& h, const Particle& p, double electron_vz) {
        h.chi2pid.fill(p.chi2pid());
        h.p.fill(p.p());
        h.vz.fill(p.vz());
        h.delta_vz.fill(p.vz() - electron_vz);
    }

    void fill_all_cut_histograms(double electron_vz,
                                  const std::vector<Particle>& protons,
                                  const std::vector<Particle>& pions_plus,
                                  const std::vector<Particle>& pions_minus,
                                  const std::vector<Particle>& kaons_plus,
                                  const std::vector<Particle>& kaons_minus) {
        for (const auto& p : protons)      fill_cut_histograms(h_exclusive.proton_cuts,  p, electron_vz);
        for (const auto& p : pions_plus)   fill_cut_histograms(h_exclusive.piplus_cuts,   p, electron_vz);
        for (const auto& p : pions_minus)  fill_cut_histograms(h_exclusive.piminus_cuts,  p, electron_vz);
        for (const auto& p : kaons_plus)   fill_cut_histograms(h_exclusive.kplus_cuts,    p, electron_vz);
        for (const auto& p : kaons_minus)  fill_cut_histograms(h_exclusive.kminus_cuts,   p, electron_vz);
    }

    bool passes_hadron_cuts(const Particle& hadron, double electron_vz) const {
        if (hadron.p() < 0.3) return false;
        if (hadron.chi2pid() < -3.0 || hadron.chi2pid() > 3.0) return false;
        if (hadron.vz() < -15.0 || hadron.vz() > 5.0) return false;
        if (std::abs(hadron.vz() - electron_vz) > 3.0) return false;
        return true;
    }

    static auto compute_exclusivity(const Particle& beam,
                                     const Particle& target,
                                     const std::vector<Particle>& detected) -> ExclusivityVars {
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

    void populate_particle_data(const Particle& particle, const RICHParticleBank& rich) {
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

    void fill_exclusive_ntuple(const Particle& hadron, int true_pid,
                               const ExclusivityVars& excl) {
        auto calo = hadron.calo();
        auto htcc = hadron.cherenkov();
        auto ftof = hadron.scintillator();

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

    void populate_calorimeter_data(const CalorimeterBank& calo) {
        auto& d = m_ntuple.data();

        // PCAL
        d.PCAL_E      = calo.pcal_energy;
        d.PCAL_chi2   = calo.pcal_chi2;
        d.PCAL_time   = calo.pcal_time;
        d.PCAL_path   = calo.pcal_path;
        d.PCAL_lu     = calo.pcal_lu;
        d.PCAL_lv     = calo.pcal_lv;
        d.PCAL_lw     = calo.pcal_lw;
        d.PCAL_du     = calo.pcal_du;
        d.PCAL_dv     = calo.pcal_dv;
        d.PCAL_dw     = calo.pcal_dw;
        d.PCAL_m2u    = calo.pcal_m2u;
        d.PCAL_m2v    = calo.pcal_m2v;
        d.PCAL_m2w    = calo.pcal_m2w;
        d.PCAL_m3u    = calo.pcal_m3u;
        d.PCAL_m3v    = calo.pcal_m3v;
        d.PCAL_m3w    = calo.pcal_m3w;
        d.PCAL_dbstU  = calo.pcal_dbstU;
        d.PCAL_dbstV  = calo.pcal_dbstV;
        d.PCAL_dbstW  = calo.pcal_dbstW;
        d.PCAL_rawEU  = calo.pcal_rawEU;
        d.PCAL_rawEV  = calo.pcal_rawEV;
        d.PCAL_rawEW  = calo.pcal_rawEW;
        d.PCAL_recEU  = calo.pcal_recEU;
        d.PCAL_recEV  = calo.pcal_recEV;
        d.PCAL_recEW  = calo.pcal_recEW;
        d.PCAL_recDTU = calo.pcal_recDTU;
        d.PCAL_recDTV = calo.pcal_recDTV;
        d.PCAL_recDTW = calo.pcal_recDTW;
        d.PCAL_recFTU = calo.pcal_recFTU;
        d.PCAL_recFTV = calo.pcal_recFTV;
        d.PCAL_recFTW = calo.pcal_recFTW;
        d.PCAL_size   = calo.pcal_size;

        // ECIN
        d.ICAL_E      = calo.inner_energy;
        d.ICAL_chi2   = calo.inner_chi2;
        d.ICAL_time   = calo.inner_time;
        d.ICAL_path   = calo.inner_path;
        d.ICAL_lu     = calo.inner_lu;
        d.ICAL_lv     = calo.inner_lv;
        d.ICAL_lw     = calo.inner_lw;
        d.ICAL_du     = calo.inner_du;
        d.ICAL_dv     = calo.inner_dv;
        d.ICAL_dw     = calo.inner_dw;
        d.ICAL_m2u    = calo.inner_m2u;
        d.ICAL_m2v    = calo.inner_m2v;
        d.ICAL_m2w    = calo.inner_m2w;
        d.ICAL_m3u    = calo.inner_m3u;
        d.ICAL_m3v    = calo.inner_m3v;
        d.ICAL_m3w    = calo.inner_m3w;
        d.ICAL_dbstU  = calo.inner_dbstU;
        d.ICAL_dbstV  = calo.inner_dbstV;
        d.ICAL_dbstW  = calo.inner_dbstW;
        d.ICAL_rawEU  = calo.inner_rawEU;
        d.ICAL_rawEV  = calo.inner_rawEV;
        d.ICAL_rawEW  = calo.inner_rawEW;
        d.ICAL_recEU  = calo.inner_recEU;
        d.ICAL_recEV  = calo.inner_recEV;
        d.ICAL_recEW  = calo.inner_recEW;
        d.ICAL_recDTU = calo.inner_recDTU;
        d.ICAL_recDTV = calo.inner_recDTV;
        d.ICAL_recDTW = calo.inner_recDTW;
        d.ICAL_recFTU = calo.inner_recFTU;
        d.ICAL_recFTV = calo.inner_recFTV;
        d.ICAL_recFTW = calo.inner_recFTW;
        d.ICAL_size   = calo.inner_size;

        // ECOUT
        d.OCAL_E      = calo.outer_energy;
        d.OCAL_chi2   = calo.outer_chi2;
        d.OCAL_time   = calo.outer_time;
        d.OCAL_path   = calo.outer_path;
        d.OCAL_lu     = calo.outer_lu;
        d.OCAL_lv     = calo.outer_lv;
        d.OCAL_lw     = calo.outer_lw;
        d.OCAL_du     = calo.outer_du;
        d.OCAL_dv     = calo.outer_dv;
        d.OCAL_dw     = calo.outer_dw;
        d.OCAL_m2u    = calo.outer_m2u;
        d.OCAL_m2v    = calo.outer_m2v;
        d.OCAL_m2w    = calo.outer_m2w;
        d.OCAL_m3u    = calo.outer_m3u;
        d.OCAL_m3v    = calo.outer_m3v;
        d.OCAL_m3w    = calo.outer_m3w;
        d.OCAL_dbstU  = calo.outer_dbstU;
        d.OCAL_dbstV  = calo.outer_dbstV;
        d.OCAL_dbstW  = calo.outer_dbstW;
        d.OCAL_rawEU  = calo.outer_rawEU;
        d.OCAL_rawEV  = calo.outer_rawEV;
        d.OCAL_rawEW  = calo.outer_rawEW;
        d.OCAL_recEU  = calo.outer_recEU;
        d.OCAL_recEV  = calo.outer_recEV;
        d.OCAL_recEW  = calo.outer_recEW;
        d.OCAL_recDTU = calo.outer_recDTU;
        d.OCAL_recDTV = calo.outer_recDTV;
        d.OCAL_recDTW = calo.outer_recDTW;
        d.OCAL_recFTU = calo.outer_recFTU;
        d.OCAL_recFTV = calo.outer_recFTV;
        d.OCAL_recFTW = calo.outer_recFTW;
        d.OCAL_size   = calo.outer_size;
    }

    void populate_cherenkov_data(const CherenkovBank& htcc) {
        auto& d = m_ntuple.data();
        d.HTCC_nphe   = htcc.nphe;
        d.HTCC_time   = htcc.time;
        d.HTCC_path   = htcc.path;
        d.HTCC_chi2   = htcc.chi2;
        d.HTCC_dtheta = htcc.dtheta;
        d.HTCC_dphi   = htcc.dphi;
    }

    void populate_scintillator_data(const ScintillatorBank& scint) {
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
            framework::H1<> p{mh::regular(0, 10, static_cast<std::size_t>(200))};
            framework::H1<> theta{mh::regular(0, 40, static_cast<std::size_t>(200))};
            framework::H1<> phi{mh::regular(-180, 180, static_cast<std::size_t>(200))};
            framework::H1<> vz{mh::regular(-30, 30, static_cast<std::size_t>(200))};
            framework::H1<> RQ{mh::regular(0, 1, static_cast<std::size_t>(200))};
            framework::H1<> best_PID{mh::regular(-400, 400, static_cast<std::size_t>(800))};
            framework::H1<> best_ntot{mh::regular(-400, 400, static_cast<std::size_t>(800))};
        } precut;

        struct {
            framework::H1<> p{mh::regular(0, 10, static_cast<std::size_t>(200))};
            framework::H1<> theta{mh::regular(0, 40, static_cast<std::size_t>(200))};
            framework::H1<> phi{mh::regular(-180, 180, static_cast<std::size_t>(200))};
            framework::H1<> vz{mh::regular(-30, 15, static_cast<std::size_t>(200))};
            framework::H1<> RQ{mh::regular(0, 1, static_cast<std::size_t>(200))};
            framework::H1<> best_PID{mh::regular(-400, 400, static_cast<std::size_t>(800))};
            framework::H1<> best_ntot{mh::regular(-400, 400, static_cast<std::size_t>(800))};
        } postcut;
    };

    HadronHistograms h_piplus;
    HadronHistograms h_piminus;
    HadronHistograms h_kplus;
    HadronHistograms h_kminus;

    void book_comparison_histograms() {
        auto* svc = m_hist.get();
        auto dir = std::string("/AiPidHadron/RICHComparison");
        svc->register_hist(dir + "/rec_vs_rich_pid", "REC PID vs RICH best_PID;REC PID;RICH best_PID", h_rich_vs_rec_pid);

        auto book_cmp = [&](PidCompareHistograms& h, const std::string& base, const std::string& label) {            svc->register_hist(base + "/rich_pid_dist", fmt::format("RICH best_PID for REC {};RICH PID;Events", label), h.rich_pid_dist);

            auto book_kin = [&](KinematicHistograms& kh, const std::string& sub, const std::string& tag) {                svc->register_hist(sub + "/p", fmt::format("{} {} Momentum;p [GeV/c];Events", label, tag), kh.p);                svc->register_hist(sub + "/theta", fmt::format("{} {} #theta;#theta [deg];Events", label, tag), kh.theta);                svc->register_hist(sub + "/phi", fmt::format("{} {} #phi;#phi [deg];Events", label, tag), kh.phi);                svc->register_hist(sub + "/beta", fmt::format("{} {} #beta;#beta;Events", label, tag), kh.beta);
            };
            book_kin(h.agree,    base + "/agree",    "(RICH=REC)");
            book_kin(h.disagree, base + "/disagree", "(RICH#neqREC)");
        };

        book_cmp(h_cmp_piplus,  dir + "/PiPlus",  "#pi^{+} S1");
        book_cmp(h_cmp_piminus, dir + "/PiMinus", "#pi^{-} S1");
        book_cmp(h_cmp_kplus,   dir + "/KPlus",   "K^{+} S1");
        book_cmp(h_cmp_kminus,  dir + "/KMinus",  "K^{-} S1");

        // Per-RICH-species REC PID distribution
        auto rdir = dir + "/ByRICH";        svc->register_hist(rdir + "/rec_pid_for_rich_piplus", "REC PID for RICH #pi^{+} S1;REC PID;Events", h_rec_pid_for_rich_piplus);        svc->register_hist(rdir + "/rec_pid_for_rich_piminus", "REC PID for RICH #pi^{-} S1;REC PID;Events", h_rec_pid_for_rich_piminus);        svc->register_hist(rdir + "/rec_pid_for_rich_kplus", "REC PID for RICH K^{+} S1;REC PID;Events", h_rec_pid_for_rich_kplus);        svc->register_hist(rdir + "/rec_pid_for_rich_kminus", "REC PID for RICH K^{-} S1;REC PID;Events", h_rec_pid_for_rich_kminus);
    }

    void fill_comparison(const Particle& particle, int rec_pid, int rich_pid) {
        PidCompareHistograms* h = nullptr;
        if      (rec_pid == static_cast<int>(PIDs::PIPLUS))  h = &h_cmp_piplus;
        else if (rec_pid == static_cast<int>(PIDs::PIMINUS)) h = &h_cmp_piminus;
        else if (rec_pid == static_cast<int>(PIDs::KPLUS))   h = &h_cmp_kplus;
        else if (rec_pid == static_cast<int>(PIDs::KMINUS))  h = &h_cmp_kminus;
        if (!h) return;

        h->rich_pid_dist.fill(rich_pid);
        h_rich_vs_rec_pid.fill(pid_to_bin_index(rec_pid), pid_to_bin_index(rich_pid));
        // Fill REC PID distribution keyed by RICH species
        if      (rich_pid == static_cast<int>(PIDs::PIPLUS) )  h_rec_pid_for_rich_piplus.fill(rec_pid);
        else if (rich_pid == static_cast<int>(PIDs::PIMINUS)) h_rec_pid_for_rich_piminus.fill(rec_pid);
        else if (rich_pid == static_cast<int>(PIDs::KPLUS)  )   h_rec_pid_for_rich_kplus.fill(rec_pid);
        else if (rich_pid == static_cast<int>(PIDs::KMINUS) )  h_rec_pid_for_rich_kminus.fill(rec_pid);

        auto& kh = (rec_pid == rich_pid) ? h->agree : h->disagree;
        kh.p.fill(particle.p());
        kh.theta.fill(particle.theta());
        kh.phi.fill(particle.phi());
        kh.beta.fill(particle.beta());
    }

    void book_histograms(const std::string& dir, HadronHistograms& h, const std::string& label) {
        auto* svc = m_hist.get();

        auto pre = dir + "/precut";        svc->register_hist(pre + "/p", fmt::format("{} Momentum (pre-cut);p [GeV/c];Events", label), h.precut.p);        svc->register_hist(pre + "/theta", fmt::format("{} #theta (pre-cut);#theta [deg];Events", label), h.precut.theta);        svc->register_hist(pre + "/phi", fmt::format("{} #phi (pre-cut);#phi [deg];Events", label), h.precut.phi);        svc->register_hist(pre + "/vz", fmt::format("{} v_{{z}} (pre-cut);v_{{z}} [cm];Events", label), h.precut.vz);        svc->register_hist(pre + "/RQ", fmt::format("{} RICH RQ (pre-cut);RQ;Events", label), h.precut.RQ);        svc->register_hist(pre + "/best_PID", fmt::format("{} RICH best PID (pre-cut);PID;Events", label), h.precut.best_PID);        svc->register_hist(pre + "/best_ntot", fmt::format("{} RICH best ntot (pre-cut);ntot;Events", label), h.precut.best_ntot);

        auto post = dir + "/postcut";        svc->register_hist(post + "/p", fmt::format("{} Momentum (post-cut);p [GeV/c];Events", label), h.postcut.p);        svc->register_hist(post + "/theta", fmt::format("{} #theta (post-cut);#theta [deg];Events", label), h.postcut.theta);        svc->register_hist(post + "/phi", fmt::format("{} #phi (post-cut);#phi [deg];Events", label), h.postcut.phi);        svc->register_hist(post + "/vz", fmt::format("{} v_{{z}} (post-cut);v_{{z}} [cm];Events", label), h.postcut.vz);        svc->register_hist(post + "/RQ", fmt::format("{} RICH RQ (post-cut);RQ;Events", label), h.postcut.RQ);        svc->register_hist(post + "/best_PID", fmt::format("{} RICH best PID (post-cut);PID;Events", label), h.postcut.best_PID);        svc->register_hist(post + "/best_ntot", fmt::format("{} RICH best ntot (post-cut);ntot;Events", label), h.postcut.best_ntot);
    }

    void fill_precut_histograms(const Particle& particle, const RICHParticleBank& rich, HadronHistograms& h) {
        h.precut.p.fill(particle.p());
        h.precut.theta.fill(particle.theta());
        h.precut.phi.fill(particle.phi());
        h.precut.vz.fill(particle.vz());
        h.precut.RQ.fill(rich.RQ);
        h.precut.best_PID.fill(rich.best_PID);
        h.precut.best_ntot.fill(rich.best_ntot);
    }

    void fill_postcut_histograms(const Particle& particle, const RICHParticleBank& rich, HadronHistograms& h) {
        h.postcut.p.fill(particle.p());
        h.postcut.theta.fill(particle.theta());
        h.postcut.phi.fill(particle.phi());
        h.postcut.vz.fill(particle.vz());
        h.postcut.RQ.fill(rich.RQ);
        h.postcut.best_PID.fill(rich.best_PID);
        h.postcut.best_ntot.fill(rich.best_ntot);
    }

    // --- Exclusive mode histograms ---
    struct ExclusiveHistograms {
        // ed -> ep pi+ pi- (n)
        framework::H1<> mm_all{mh::regular(0.0, 2.0, static_cast<std::size_t>(300))};
        framework::H1<> mm_selected{mh::regular(0.0, 2.0, static_cast<std::size_t>(300))};
        framework::H1<> mm_squared{mh::regular(-1.0, 3.0, static_cast<std::size_t>(300))};
        framework::H1<> n_combinations{mh::regular(0, 20, static_cast<std::size_t>(20))};
        framework::H1<> piplus_p{mh::regular(0, 3, static_cast<std::size_t>(200))};
        framework::H1<> piplus_theta{mh::regular(0, 180, static_cast<std::size_t>(200))};
        framework::H1<> piminus_p{mh::regular(0, 3, static_cast<std::size_t>(200))};
        framework::H1<> piminus_theta{mh::regular(0, 180, static_cast<std::size_t>(200))};

        // ed -> ep K+ K- (n)
        framework::H1<> kaon_mm_all{mh::regular(0.0, 2.0, static_cast<std::size_t>(300))};
        framework::H1<> kaon_mm_selected{mh::regular(0.0, 2.0, static_cast<std::size_t>(300))};
        framework::H1<> kaon_mm_squared{mh::regular(-1.0, 3.0, static_cast<std::size_t>(300))};
        framework::H1<> kaon_n_combinations{mh::regular(0, 20, static_cast<std::size_t>(20))};
        framework::H1<> kplus_p{mh::regular(0, 3, static_cast<std::size_t>(200))};
        framework::H1<> kplus_theta{mh::regular(0, 180, static_cast<std::size_t>(200))};
        framework::H1<> kminus_p{mh::regular(0, 3, static_cast<std::size_t>(200))};
        framework::H1<> kminus_theta{mh::regular(0, 180, static_cast<std::size_t>(200))};

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
        auto* svc = m_hist.get();
        auto dir = std::string("/AiPidHadron/Exclusive");
        svc->register_hist(dir + "/missing_mass_all", "Missing Mass ed#rightarrowep#pi^{+}#pi^{-}(X) (all combos);MM [GeV/c^{2}];Events", h_exclusive.mm_all);
        svc->register_hist(dir + "/missing_mass_selected", "Missing Mass ed#rightarrowep#pi^{+}#pi^{-}(X) (selected);MM [GeV/c^{2}];Events", h_exclusive.mm_selected);
        svc->register_hist(dir + "/missing_mass_squared", "Missing Mass^{2} ed#rightarrowep#pi^{+}#pi^{-}(X);MM^{2} [GeV^{2}/c^{4}];Events", h_exclusive.mm_squared);
        svc->register_hist(dir + "/n_combinations", "Number of (p,#pi^{+},#pi^{-}) combinations per event;N_{combos};Events", h_exclusive.n_combinations);
        svc->register_hist(dir + "/piplus_momentum", "#pi^{+} Momentum (exclusive);p [GeV/c];Events", h_exclusive.piplus_p);
        svc->register_hist(dir + "/piplus_theta", "#pi^{+} #theta (exclusive);#theta [deg];Events", h_exclusive.piplus_theta);
        svc->register_hist(dir + "/piminus_momentum", "#pi^{-} Momentum (exclusive);p [GeV/c];Events", h_exclusive.piminus_p);
        svc->register_hist(dir + "/piminus_theta", "#pi^{-} #theta (exclusive);#theta [deg];Events", h_exclusive.piminus_theta);

        // --- ed -> ep K+ K- (n) ---
        auto kdir = std::string("/AiPidHadron/ExclusiveKaon");
        svc->register_hist(kdir + "/missing_mass_all", "Missing Mass ed#rightarrowepK^{+}K^{-}(X) (all combos);MM [GeV/c^{2}];Events", h_exclusive.kaon_mm_all);
        svc->register_hist(kdir + "/missing_mass_selected", "Missing Mass ed#rightarrowepK^{+}K^{-}(X) (selected);MM [GeV/c^{2}];Events", h_exclusive.kaon_mm_selected);
        svc->register_hist(kdir + "/missing_mass_squared", "Missing Mass^{2} ed#rightarrowepK^{+}K^{-}(X);MM^{2} [GeV^{2}/c^{4}];Events", h_exclusive.kaon_mm_squared);
        svc->register_hist(kdir + "/n_combinations", "Number of (p,K^{+},K^{-}) combinations per event;N_{combos};Events", h_exclusive.kaon_n_combinations);
        svc->register_hist(kdir + "/kplus_momentum", "K^{+} Momentum (exclusive);p [GeV/c];Events", h_exclusive.kplus_p);
        svc->register_hist(kdir + "/kplus_theta", "K^{+} #theta (exclusive);#theta [deg];Events", h_exclusive.kplus_theta);
        svc->register_hist(kdir + "/kminus_momentum", "K^{-} Momentum (exclusive);p [GeV/c];Events", h_exclusive.kminus_p);
        svc->register_hist(kdir + "/kminus_theta", "K^{-} #theta (exclusive);#theta [deg];Events", h_exclusive.kminus_theta);

        // --- Exclusivity histograms ---
        auto book_excl_hists = [&](ExclusivityHistograms& eh, const std::string& base, const std::string& label) {            svc->register_hist(base + "/missing_energy", fmt::format("{} Missing Energy;#DeltaE [GeV];Events", label), eh.missing_energy);            svc->register_hist(base + "/missing_momentum", fmt::format("{} Missing Momentum;|p_{{miss}}| [GeV/c];Events", label), eh.missing_p);            svc->register_hist(base + "/missing_pt", fmt::format("{} Missing p_{{T}};p_{{T,miss}} [GeV/c];Events", label), eh.missing_pt);            svc->register_hist(base + "/missing_theta", fmt::format("{} Missing #theta;#theta_{{miss}} [deg];Events", label), eh.missing_theta);            svc->register_hist(base + "/missing_phi", fmt::format("{} Missing #phi;#phi_{{miss}} [deg];Events", label), eh.missing_phi);
            // Neutron detection histograms
            auto ndir = base + "/Neutron";            svc->register_hist(ndir + "/n_detected", fmt::format("{} Number of detected neutrons;N_{{n}};Events", label), eh.n_detected);            svc->register_hist(ndir + "/opening_angle", fmt::format("{} Neutron-Missing opening angle;#Delta#alpha [deg];Events", label), eh.neutron_opening_angle);            svc->register_hist(ndir + "/neutron_p", fmt::format("{} Detected neutron momentum;p_{{n}} [GeV/c];Events", label), eh.neutron_p);            svc->register_hist(ndir + "/neutron_beta", fmt::format("{} Detected neutron #beta;#beta_{{n}};Events", label), eh.neutron_beta);            svc->register_hist(ndir + "/mm_with_neutron", fmt::format("{} MM (with detected n);MM [GeV/c^{{2}}];Events", label), eh.mm_with_neutron);
        };

        book_excl_hists(h_exclusive.pion_excl, dir, "ed#rightarrowep#pi^{+}#pi^{-}(X)");
        book_excl_hists(h_exclusive.kaon_excl, kdir, "ed#rightarrowepK^{+}K^{-}(X)");

        // --- Hadron cut variable histograms ---
        auto book_cut_hists = [&](ParticleCutHistograms& h, const std::string& base, const std::string& label) {            svc->register_hist(base + "/chi2pid", fmt::format("{} #chi^{{2}}_{{PID}};#chi^{{2}}_{{PID}};Events", label), h.chi2pid);            svc->register_hist(base + "/p", fmt::format("{} Momentum;p [GeV/c];Events", label), h.p);            svc->register_hist(base + "/vz", fmt::format("{} v_{{z}};v_{{z}} [cm];Events", label), h.vz);            svc->register_hist(base + "/delta_vz", fmt::format("{} #Deltav_{{z}};#Deltav_{{z}} [cm];Events", label), h.delta_vz);
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
