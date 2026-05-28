#pragma once

#include <Math/VectorUtil.h>
#include <bank.h>
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

namespace framework {

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
 * auto photon_cuts = service<framework::PhotonCutsService>("PhotonCutsService");
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
        const hipo::bank& RUN_config,
        const hipo::bank& REC_Particle,
        const hipo::bank& REC_Calorimeter) const -> std::vector<Particle> {

        int run_number = RUN_config.getInt("run", 0);
        std::vector<int> good_photon_rows;
        auto calo_map = get_calo_map(REC_Calorimeter);

        for (int i = 0; i < REC_Particle.getRows(); i++) {
            int pid = REC_Particle.getInt("pid", i);
            if (pid == 22) {
                if (filter_photon(REC_Particle, calo_map, i, run_number, m_pass)) {
                    good_photon_rows.push_back(i);
                }
            }
        }

        // Build particle objects for good photons
        std::vector<Particle> good_photons;
        for (const auto& row : good_photon_rows) {
            int pid = REC_Particle.getInt("pid", row);
            int status = REC_Particle.getShort("status", row);
            int charge = REC_Particle.getByte("charge", row);

            double px = static_cast<double>(REC_Particle.getFloat("px", row));
            double py = static_cast<double>(REC_Particle.getFloat("py", row));
            double pz = static_cast<double>(REC_Particle.getFloat("pz", row));
            double vx = static_cast<double>(REC_Particle.getFloat("vx", row));
            double vy = static_cast<double>(REC_Particle.getFloat("vy", row));
            double vz = static_cast<double>(REC_Particle.getFloat("vz", row));
            double vt = static_cast<double>(REC_Particle.getFloat("vt", row));
            double beta = static_cast<double>(REC_Particle.getFloat("beta", row));
            double chi2pid = static_cast<double>(REC_Particle.getFloat("chi2pid", row));
            double E = physics::ParticleUtils::compute_energy(px, py, pz, pid);

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

    static auto get_calo_map(const hipo::bank& bank) -> std::map<int, CaloRowData> {
        std::map<int, CaloRowData> calo_map;

        for (int row = 0; row < bank.getRows(); row++) {
            auto pindex = bank.getShort("pindex", row);
            auto x = bank.getFloat("x", row);
            auto y = bank.getFloat("y", row);
            auto z = bank.getFloat("z", row);
            auto m2u = bank.getFloat("m2u", row);
            auto m2v = bank.getFloat("m2v", row);
            auto layer = static_cast<int>(bank.getByte("layer", row));
            auto e = bank.getFloat("energy", row);

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
        const hipo::bank& particle_bank,
        const std::map<int, CaloRowData>& calo_map,
        int row,
        int runnum,
        int pass) const -> bool {

        // Set variables native to the photon we are classifying
        double gPx = static_cast<double>(particle_bank.getFloat("px", row));
        double gPy = static_cast<double>(particle_bank.getFloat("py", row));
        double gPz = static_cast<double>(particle_bank.getFloat("pz", row));

        // Set ML features intrinsic to the photon of interest
        double gE = std::sqrt(gPx * gPx + gPy * gPy + gPz * gPz);
        double gTheta = std::acos(gPz / gE);

        auto calo_it = calo_map.find(row);
        if (calo_it == calo_map.end()) return false;

        const auto& calo_POI = calo_it->second;
        double gEpcal = calo_POI.pcal_e;
        double gm2u = calo_POI.pcal_m2u;
        double gm2v = calo_POI.pcal_m2v;

        // Apply PID purity cuts on the photon
        if (!pass_pid_purity_filter(gE, gEpcal, gTheta)) return false;

        // Define the variables for nearest neighbors
        constexpr int m_g = 3;   // Number of neighboring gammas
        constexpr int m_ch = 2;  // Number of neighboring charged hadrons
        constexpr int m_nh = 2;  // Number of neighboring neutral hadrons

        double R_e = 0;
        double dE_e = 0;

        double R_gamma[m_g] = {0};
        double dE_gamma[m_g] = {0};
        double Epcal_gamma[m_g] = {0};
        double m2u_gamma[m_g] = {0};
        double m2v_gamma[m_g] = {0};

        double R_ch[m_ch] = {0};
        double dE_ch[m_ch] = {0};
        double Epcal_ch[m_ch] = {0};
        double m2u_ch[m_ch] = {0};
        double m2v_ch[m_ch] = {0};

        double R_nh[m_nh] = {0};
        double dE_nh[m_nh] = {0};
        double Epcal_nh[m_nh] = {0};
        double m2u_nh[m_nh] = {0};
        double m2v_nh[m_nh] = {0};

        double num_photons_0_1 = 0;
        double num_photons_0_2 = 0;
        double num_photons_0_35 = 0;

        // Get 3-vector for photon of interest's calorimeter location
        ROOT::Math::XYZVector vPOI = get_particle_calo_vector(calo_POI);

        // Build nearest neighbor event structure
        for (int inner_row = 0; inner_row < particle_bank.getRows(); inner_row++) {
            if (inner_row == row) continue;

            auto inner_calo_it = calo_map.find(inner_row);
            if (inner_calo_it == calo_map.end()) continue;

            const auto& calo_PART = inner_calo_it->second;

            auto pid = particle_bank.getInt("pid", inner_row);
            auto mass = physics::ParticleUtils::get_mass(pid);

            if (std::isnan(mass)) continue;

            auto px = particle_bank.getFloat("px", inner_row);
            auto py = particle_bank.getFloat("py", inner_row);
            auto pz = particle_bank.getFloat("pz", inner_row);
            auto p = std::sqrt(px * px + py * py + pz * pz);
            auto E = std::sqrt(p * p + mass * mass);
            auto th = std::acos(pz / p);

            // Skip if not in forward detector
            double th_deg = th * 180.0 / M_PI;
            if (th_deg < MIN_FD_THETA || th_deg > MAX_FD_THETA) continue;

            ROOT::Math::XYZVector vPART = get_particle_calo_vector(calo_PART);
            double R = ROOT::Math::VectorUtil::Angle(vPOI, vPART);

            if (pid == 22) {  // photon
                if (!pass_pid_purity_filter(E, calo_PART.pcal_e, th)) continue;

                if (R < 0.1) num_photons_0_1++;
                if (R < 0.2) num_photons_0_2++;
                if (R < 0.35) num_photons_0_35++;

                for (int i = 0; i < m_g; ++i) {
                    if (R < R_gamma[i] || R_gamma[i] == 0) {
                        for (int j = m_g - 1; j > i; --j) {
                            R_gamma[j] = R_gamma[j - 1];
                            dE_gamma[j] = dE_gamma[j - 1];
                            Epcal_gamma[j] = Epcal_gamma[j - 1];
                            m2u_gamma[j] = m2u_gamma[j - 1];
                            m2v_gamma[j] = m2v_gamma[j - 1];
                        }
                        R_gamma[i] = R;
                        dE_gamma[i] = gE - E;
                        Epcal_gamma[i] = calo_PART.pcal_e;
                        m2u_gamma[i] = calo_PART.pcal_m2u;
                        m2v_gamma[i] = calo_PART.pcal_m2v;
                        break;
                    }
                }
            } else if (pid == 11) {  // electron
                if (R < R_e || R_e == 0) {
                    R_e = R;
                    dE_e = gE - E;
                }
            } else if (pid == 2212 || pid == -2212 || pid == 211 || pid == -211 || pid == 321 || pid == -321) {
                // charged hadron
                for (int i = 0; i < m_ch; ++i) {
                    if (R < R_ch[i] || R_ch[i] == 0) {
                        for (int j = m_ch - 1; j > i; --j) {
                            R_ch[j] = R_ch[j - 1];
                            dE_ch[j] = dE_ch[j - 1];
                            Epcal_ch[j] = Epcal_ch[j - 1];
                            m2u_ch[j] = m2u_ch[j - 1];
                            m2v_ch[j] = m2v_ch[j - 1];
                        }
                        R_ch[i] = R;
                        dE_ch[i] = gE - E;
                        Epcal_ch[i] = calo_PART.pcal_e;
                        m2u_ch[i] = calo_PART.pcal_m2u;
                        m2v_ch[i] = calo_PART.pcal_m2v;
                        break;
                    }
                }
            } else if (pid == 2112 || pid == -2112) {  // neutral hadron
                for (int i = 0; i < m_nh; ++i) {
                    if (R < R_nh[i] || R_nh[i] == 0) {
                        for (int j = m_nh - 1; j > i; --j) {
                            R_nh[j] = R_nh[j - 1];
                            dE_nh[j] = dE_nh[j - 1];
                            Epcal_nh[j] = Epcal_nh[j - 1];
                            m2u_nh[j] = m2u_nh[j - 1];
                            m2v_nh[j] = m2v_nh[j - 1];
                        }
                        R_nh[i] = R;
                        dE_nh[i] = gE - E;
                        Epcal_nh[i] = calo_PART.pcal_e;
                        m2u_nh[i] = calo_PART.pcal_m2u;
                        m2v_nh[i] = calo_PART.pcal_m2v;
                        break;
                    }
                }
            }
        }

        // Create input_data vector for the ML model
        std::vector<float> input_data = {
            static_cast<float>(gE), static_cast<float>(gEpcal), static_cast<float>(gTheta),
            static_cast<float>(gm2u), static_cast<float>(gm2v), static_cast<float>(R_e),
            static_cast<float>(dE_e)};

        for (int i = 0; i < m_g; ++i) input_data.push_back(static_cast<float>(R_gamma[i]));
        for (int i = 0; i < m_g; ++i) input_data.push_back(static_cast<float>(dE_gamma[i]));
        for (int i = 0; i < m_g; ++i) input_data.push_back(static_cast<float>(Epcal_gamma[i]));
        for (int i = 0; i < m_g; ++i) input_data.push_back(static_cast<float>(m2u_gamma[i]));
        for (int i = 0; i < m_g; ++i) input_data.push_back(static_cast<float>(m2v_gamma[i]));

        for (int i = 0; i < m_ch; ++i) input_data.push_back(static_cast<float>(R_ch[i]));
        for (int i = 0; i < m_ch; ++i) input_data.push_back(static_cast<float>(dE_ch[i]));
        for (int i = 0; i < m_ch; ++i) input_data.push_back(static_cast<float>(Epcal_ch[i]));
        for (int i = 0; i < m_ch; ++i) input_data.push_back(static_cast<float>(m2u_ch[i]));
        for (int i = 0; i < m_ch; ++i) input_data.push_back(static_cast<float>(m2v_ch[i]));

        for (int i = 0; i < m_nh; ++i) input_data.push_back(static_cast<float>(R_nh[i]));
        for (int i = 0; i < m_nh; ++i) input_data.push_back(static_cast<float>(dE_nh[i]));
        for (int i = 0; i < m_nh; ++i) input_data.push_back(static_cast<float>(Epcal_nh[i]));
        for (int i = 0; i < m_nh; ++i) input_data.push_back(static_cast<float>(m2u_nh[i]));
        for (int i = 0; i < m_nh; ++i) input_data.push_back(static_cast<float>(m2v_nh[i]));

        input_data.push_back(static_cast<float>(num_photons_0_1));
        input_data.push_back(static_cast<float>(num_photons_0_2));
        input_data.push_back(static_cast<float>(num_photons_0_35));

        return classify_photon(input_data, runnum, pass);
    }
};

}  // namespace framework
