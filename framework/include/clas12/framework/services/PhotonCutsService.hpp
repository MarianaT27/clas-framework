#pragma once

#include <Math/VectorUtil.h>
#include <bank_view.h>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/models/RGA_inbending_pass1.hpp>
#include <clas12/models/RGA_inbending_pass2.hpp>
#include <clas12/models/RGA_outbending_pass1.hpp>
#include <clas12/models/RGA_outbending_pass2.hpp>
#include <clas12/models/RGC_Summer2022_pass1.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/physics/ParticleUtils.hpp>

#include <cmath>
#include <functional>
#include <map>
#include <tuple>
#include <vector>

namespace clas12::framework {

/**
 * @brief Service for photon selection cuts in CLAS12 analysis
 *
 * This service provides photon identification using:
 * - AI/ML-based photon classification (GBT models)
 * - PCAL fiducial cuts
 * - Run-dependent model selection
 *
 * Usage:
 * @code
 * auto photon_cuts = service<clas12::framework::PhotonCutsService>("PhotonCutsService");
 * photon_cuts->set_pass(1);  // Set reconstruction pass
 *
 * auto good_photons = photon_cuts->select_photons_with_AI(
 *     RUN_config, REC_Particle, REC_Calorimeter);
 * @endcode
 */
class PhotonCutsService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "PhotonCutsService"; }

    // ============================================================================
    // Constants
    // ============================================================================

    static constexpr double AI_THRESHOLD = 0.78;      // Threshold for model predictions
    static constexpr double MIN_PCAL_V = 14.0;        // cm
    static constexpr double MIN_PCAL_W = 14.0;        // cm
    static constexpr double MIN_PHOTON_ENERGY = 0.2;  // GeV
    static constexpr double MIN_FD_THETA = 5.0;       // degrees
    static constexpr double MAX_FD_THETA = 35.0;      // degrees

    // ============================================================================
    // Service Interface
    // ============================================================================

    explicit PhotonCutsService(std::string name = "PhotonCutsService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    // ============================================================================
    // Configuration
    // ============================================================================

    void set_pass(int pass) { m_pass = pass; }

    void set_ai_threshold(double threshold) { m_ai_threshold = threshold; }

    [[nodiscard]] auto pass() const -> int { return m_pass; }

    [[nodiscard]] auto ai_threshold() const -> double { return m_ai_threshold; }

    // ============================================================================
    // Individual Cut Functions
    // ============================================================================

    [[nodiscard]] static auto pass_pcal_fiducial_cut(double v, double w) -> bool {
        return v > MIN_PCAL_V && w > MIN_PCAL_W;
    }

    [[nodiscard]] static auto pass_forward_detector_cut(double theta_deg) -> bool {
        return theta_deg >= MIN_FD_THETA && theta_deg <= MAX_FD_THETA;
    }

    [[nodiscard]] static auto pass_energy_cut(double E) -> bool {
        return E >= MIN_PHOTON_ENERGY;
    }

    // ============================================================================
    // AI-Based Photon Selection
    // ============================================================================

    /**
     * @brief Select photons using AI/ML classification
     *
     * @param RUN_config RUN::config bank
     * @param REC_Particle REC::Particle bank
     * @param REC_Calorimeter REC::Calorimeter bank
     * @return Vector of good photon particles
     */
    auto select_photons_with_AI(
        const hipo::bank_view& RUN_config,
        const hipo::bank_view& REC_Particle,
        const hipo::bank_view& REC_Calorimeter) const -> std::vector<clas12::Particle> {

        int run_number = RUN_config.get<int32_t>("run", 0);
        std::vector<int> good_photon_rows;
        auto calo_map = get_calo_map(REC_Calorimeter);

        for (int i = 0; i < REC_Particle.rows(); i++) {
            int pid = REC_Particle.get<int32_t>("pid", i);
            if (pid == 22) {
                if (filter_photon(REC_Particle, calo_map, i, run_number, m_pass)) {
                    good_photon_rows.push_back(i);
                }
            }
        }

        // Build particle objects for good photons
        std::vector<clas12::Particle> good_photons;
        for (const auto& row : good_photon_rows) {
            int pid = REC_Particle.get<int32_t>("pid", row);
            int status = static_cast<int>(REC_Particle.get<int16_t>("status", row));
            int charge = static_cast<int>(REC_Particle.get<int8_t>("charge", row));

            double px = static_cast<double>(REC_Particle.get<float>("px", row));
            double py = static_cast<double>(REC_Particle.get<float>("py", row));
            double pz = static_cast<double>(REC_Particle.get<float>("pz", row));
            double vx = static_cast<double>(REC_Particle.get<float>("vx", row));
            double vy = static_cast<double>(REC_Particle.get<float>("vy", row));
            double vz = static_cast<double>(REC_Particle.get<float>("vz", row));
            double vt = static_cast<double>(REC_Particle.get<float>("vt", row));
            double beta = static_cast<double>(REC_Particle.get<float>("beta", row));
            double chi2pid = static_cast<double>(REC_Particle.get<float>("chi2pid", row));
            double E = clas12::physics::ParticleUtils::compute_energy(px, py, pz, pid);

            good_photons.emplace_back(pid, status, row, charge, 0, px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
        }

        return good_photons;
    }

   private:
    int m_pass = 1;
    double m_ai_threshold = AI_THRESHOLD;

    // ============================================================================
    // Internal Structures
    // ============================================================================

    struct CaloRowData {
        double pcal_x = 0, pcal_y = 0, pcal_z = 0;
        double ecin_x = 0, ecin_y = 0, ecin_z = 0;
        double ecout_x = 0, ecout_y = 0, ecout_z = 0;
        double pcal_e = 0, pcal_m2u = 0, pcal_m2v = 0;
        double ecin_e = 0, ecin_m2u = 0, ecin_m2v = 0;
        double ecout_e = 0, ecout_m2u = 0, ecout_m2v = 0;
    };

    // clang-format off
    // Map for the GBT Models to use depending on pass and run number
    static inline std::map<std::tuple<int, int, int>, std::function<double(std::vector<float> const&)>> const s_model_map = {
        {{5032, 5332, 1}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }},
        {{5032, 5332, 2}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }},
        {{5333, 5666, 1}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_outbending_pass1(data); }},
        {{5333, 5666, 2}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_outbending_pass2(data); }},
        {{6616, 6783, 1}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }},
        {{6616, 6783, 2}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }},
        {{6156, 6603, 1}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }},
        {{6156, 6603, 2}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }},
        {{11093, 11283, 1}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_outbending_pass1(data); }},
        {{11093, 11283, 2}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_outbending_pass2(data); }},
        {{11284, 11300, 1}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }},
        {{11284, 11300, 2}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }},
        {{11323, 11571, 1}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass1(data); }},
        {{11323, 11571, 2}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGA_inbending_pass2(data); }},
        {{16042, 16772, 1}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGC_Summer2022_pass1(data); }},
        {{16042, 16772, 2}, [](std::vector<float> const& data) { return ApplyCatboostModel_RGC_Summer2022_pass1(data); }}
    };
    // clang-format on

    // ============================================================================
    // Internal Helper Functions
    // ============================================================================

    static auto get_calo_map(const hipo::bank_view& bank) -> std::map<int, CaloRowData> {
        std::map<int, CaloRowData> calo_map;

        for (int row = 0; row < bank.rows(); row++) {
            auto pindex = bank.get<int16_t>("pindex", row);
            auto x = bank.get<float>("x", row);
            auto y = bank.get<float>("y", row);
            auto z = bank.get<float>("z", row);
            auto m2u = bank.get<float>("m2u", row);
            auto m2v = bank.get<float>("m2v", row);
            auto layer = static_cast<int>(bank.get<int8_t>("layer", row));
            auto e = bank.get<float>("energy", row);

            if (calo_map.find(pindex) == calo_map.end()) {
                calo_map[pindex] = CaloRowData();
            }

            switch (layer) {
                case 1:  // pcal
                    calo_map[pindex].pcal_x = x;
                    calo_map[pindex].pcal_y = y;
                    calo_map[pindex].pcal_z = z;
                    calo_map[pindex].pcal_e = e;
                    calo_map[pindex].pcal_m2u = m2u;
                    calo_map[pindex].pcal_m2v = m2v;
                    break;
                case 4:  // ecin
                    calo_map[pindex].ecin_x = x;
                    calo_map[pindex].ecin_y = y;
                    calo_map[pindex].ecin_z = z;
                    break;
                case 7:  // ecout
                    calo_map[pindex].ecout_x = x;
                    calo_map[pindex].ecout_y = y;
                    calo_map[pindex].ecout_z = z;
                    break;
            }
        }
        return calo_map;
    }

    static auto get_particle_calo_vector(const CaloRowData& calo_row) -> ROOT::Math::XYZVector {
        ROOT::Math::XYZVector v;
        if (calo_row.pcal_x == 0) {
            if (calo_row.ecin_x == 0) {
                v.SetXYZ(calo_row.ecout_x, calo_row.ecout_y, calo_row.ecout_z);
            } else {
                v.SetXYZ(calo_row.ecin_x, calo_row.ecin_y, calo_row.ecin_z);
            }
        } else {
            v.SetXYZ(calo_row.pcal_x, calo_row.pcal_y, calo_row.pcal_z);
        }
        return v;
    }

    static auto pass_pid_purity_filter(double E, double Epcal, double theta) -> bool {
        double theta_deg = theta * 180.0 / M_PI;
        return E >= MIN_PHOTON_ENERGY && Epcal > 0 &&
               theta_deg >= MIN_FD_THETA && theta_deg <= MAX_FD_THETA;
    }

    static auto get_model_function(int runnum, int pass)
        -> std::function<double(std::vector<float> const&)> {

        for (const auto& entry : s_model_map) {
            if (runnum >= std::get<0>(entry.first) &&
                runnum <= std::get<1>(entry.first) &&
                pass == std::get<2>(entry.first)) {
                return entry.second;
            }
        }

        // Default model
        return [](std::vector<float> const& data) {
            return ApplyCatboostModel_RGA_inbending_pass1(data);
        };
    }

    auto classify_photon(const std::vector<float>& input_data, int runnum, int pass) const -> bool {
        auto model_func = get_model_function(runnum, pass);
        double sigmoid_x = model_func(input_data);
        double prediction = 1.0 / (1.0 + std::exp(-sigmoid_x));
        return prediction > m_ai_threshold;
    }

    auto filter_photon(
        const hipo::bank_view& particle_bank,
        const std::map<int, CaloRowData>& calo_map,
        int row,
        int runnum,
        int pass) const -> bool;
};

}  // namespace clas12::framework
