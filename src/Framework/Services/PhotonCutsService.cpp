#include "Framework/Services/PhotonCutsService.hpp"

namespace Framework {

auto PhotonCutsService::filter_photon(
    const hipo::bank& particle_bank,
    const std::map<int, CaloRowData>& calo_map,
    int row,
    int runnum,
    int pass) const -> bool {

    // Set variables native to the photon we are classifying
    double gPx = particle_bank.getFloat("px", row);
    double gPy = particle_bank.getFloat("py", row);
    double gPz = particle_bank.getFloat("pz", row);

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
        auto mass = Physics::ParticleUtils::get_mass(pid);

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

}  // namespace Framework
