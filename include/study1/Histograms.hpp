#pragma once

#include <TH1.h>
#include <TH2.h>
#include <ROOT/TThreadedObject.hxx>
#include <atomic>

using TTO_TH1D = ROOT::TThreadedObject<TH1D>;
using TTO_TH2D = ROOT::TThreadedObject<TH2D>;
using up_TTO_TH1D = std::unique_ptr<TTO_TH1D>;
using up_TTO_TH2D = std::unique_ptr<TTO_TH2D>;

namespace study1 {

inline up_TTO_TH1D make_hist1d(const std::string& name, const std::string& title, int bins, double xmin, double xmax) {
    return std::make_unique<TTO_TH1D>(name.c_str(), title.c_str(), bins, xmin, xmax);
}

inline up_TTO_TH2D make_hist2d(const std::string& name, const std::string& title, int xbins, double xmin, double xmax, int ybins, double ymin, double ymax) {
    return std::make_unique<TTO_TH2D>(name.c_str(), title.c_str(), xbins, xmin, xmax, ybins, ymin, ymax);
}

struct Electron {
    up_TTO_TH1D hist1D_p = std::make_unique<TTO_TH1D>("p_e", "", 200, 0, 11);
    up_TTO_TH1D hist1D_phi = std::make_unique<TTO_TH1D>("phi_e", "", 200, -190, 190);
    up_TTO_TH1D hist1D_theta = std::make_unique<TTO_TH1D>("theta_e", "", 200, 0, 35);
    up_TTO_TH1D hist1D_chi2 = std::make_unique<TTO_TH1D>("chi2_e", "", 200, -6, 6);
    up_TTO_TH1D hist1D_vz = std::make_unique<TTO_TH1D>("vz_e", "", 200, -30, 20);

    up_TTO_TH1D hist1D_p_cut = std::make_unique<TTO_TH1D>("p_e_cut", "", 200, 0, 11);
    up_TTO_TH1D hist1D_phi_cut = std::make_unique<TTO_TH1D>("phi_e_cut", "", 200, -190, 190);
    up_TTO_TH1D hist1D_theta_cut = std::make_unique<TTO_TH1D>("theta_e_cut", "", 200, 0, 35);
    up_TTO_TH1D hist1D_chi2_cut = std::make_unique<TTO_TH1D>("chi2_e_cut", "", 200, -6, 6);
    up_TTO_TH1D hist1D_vz_cut = std::make_unique<TTO_TH1D>("vz_e_cut", "", 200, -30, 20);

    up_TTO_TH2D hist2D_lu_vs_lv_e_pcal = std::make_unique<TTO_TH2D>("lu_vs_lv_e_pcal", "", 200, 0, 450, 200, 0, 450);
    up_TTO_TH2D hist2D_lu_vs_lw_e_pcal = std::make_unique<TTO_TH2D>("lu_vs_lw_e_pcal", "", 200, 0, 450, 200, 0, 450);
    up_TTO_TH2D hist2D_lu_vs_lv_e_in = std::make_unique<TTO_TH2D>("lu_vs_lv_e_in", "", 200, 0, 450, 200, 0, 450);
    up_TTO_TH2D hist2D_lu_vs_lw_e_in = std::make_unique<TTO_TH2D>("lu_vs_lw_e_in", "", 200, 0, 450, 200, 0, 450);
    up_TTO_TH2D hist2D_lu_vs_lv_e_out = std::make_unique<TTO_TH2D>("lu_vs_lv_e_out", "", 200, 0, 450, 200, 0, 450);
    up_TTO_TH2D hist2D_lu_vs_lw_e_out = std::make_unique<TTO_TH2D>("lu_vs_lw_e_out", "", 200, 0, 450, 200, 0, 450);

    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_1_2 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_1_2", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_2_3 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_2_3", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_3_4 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_3_4", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_4_5 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_4_5", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_5_6 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_5_6", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_6_7 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_6_7", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_7_8 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_7_8", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_8_9 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_8_9", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_9_10 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_9_10", "", 200, 0, 50, 200, 0, 100);

    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_1_2 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_1_2", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_2_3 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_2_3", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_3_4 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_3_4", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_4_5 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_4_5", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_5_6 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_5_6", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_6_7 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_6_7", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_7_8 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_7_8", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_8_9 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_8_9", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_9_10 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_9_10", "", 200, 0, 50, 200, 0, 100);

    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_1_2 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_1_2", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_2_3 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_2_3", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_3_4 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_3_4", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_4_5 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_4_5", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_5_6 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_5_6", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_6_7 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_6_7", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_7_8 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_7_8", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_8_9 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_8_9", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_9_10 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_9_10", "", 200, 0, 50, 200, 0, 100);

    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_t5_10 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_t5_10", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_t10_15 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_t10_15", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_t15_20 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_t15_20", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_t20_25 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_t20_25", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_t25_30 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_t25_30", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r1_t30_35 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r1_t30_35", "", 200, 0, 50, 200, 0, 100);


    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_t5_10 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_t5_10", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_t10_15 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_t10_15", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_t15_20 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_t15_20", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_t20_25 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_t20_25", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_t25_30 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_t25_30", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r2_t30_35 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r2_t30_35", "", 200, 0, 50, 200, 0, 100);


    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_t5_10 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_t5_10", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_t10_15 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_t10_15", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_t15_20 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_t15_20", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_t20_25 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_t20_25", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_t25_30 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_t25_30", "", 200, 0, 50, 200, 0, 100);
    up_TTO_TH2D hist2D_chi2ndf_vs_edge_r3_t30_35 = std::make_unique<TTO_TH2D>("chi2ndf_vs_edge_r3_t30_35", "", 200, 0, 50, 200, 0, 100);

};

struct PionMinus {
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_pim = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_pi_minus", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_km = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_k_minus", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_electron = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_electron", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_photon = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_photon", "", 200, 0, 11, 200, -1, 1);

    up_TTO_TH2D hist2D_delta_t_vs_p_reconstructed_pim = std::make_unique<TTO_TH2D>("delta_t_vs_p_reconstructed_pi_minus", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_pim_in_recon = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_pi_minus_in_recon", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_km_in_recon = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_k_minus_in_recon", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_electron_in_recon = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_electron_in_recon", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_photon_in_recon = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_photon_in_recon", "", 200, 0, 11, 200, -1, 1);

    up_TTO_TH2D hist2D_delta_t_vs_p_matched_pim_cut = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_pi_minus_cut", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_km_cut = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_k_minus_cut", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_electron_cut = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_electron_cut", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_photon_cut = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_photon_cut", "", 200, 0, 11, 200, -1, 1);

    up_TTO_TH2D hist2D_delta_t_vs_p_reconstructed_pim_cut = std::make_unique<TTO_TH2D>("delta_t_vs_p_reconstructed_pi_minus_cut", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_pim_in_recon_cut = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_pi_minus_in_recon_cut", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_km_in_recon_cut = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_k_minus_in_recon_cut", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_electron_in_recon_cut = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_electron_in_recon_cut", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_photon_in_recon_cut = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_photon_in_recon_cut", "", 200, 0, 11, 200, -1, 1);

    up_TTO_TH2D hist2D_delta_t_vs_p_rich_matched_pim = std::make_unique<TTO_TH2D>("delta_t_vs_p_rich_matched_pi_minus", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_rich_matched_km = std::make_unique<TTO_TH2D>("delta_t_vs_p_rich_matched_k_minus", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_rich_matched_electron = std::make_unique<TTO_TH2D>("delta_t_vs_p_rich_matched_electron", "", 200, 0, 11, 200, -1, 1);

    up_TTO_TH1D hist1D_p_electron = std::make_unique<TTO_TH1D>("p_electron", "", 200, 0, 11);
    up_TTO_TH1D hist1D_p_pim = std::make_unique<TTO_TH1D>("p_pi_minus", "", 200, 0, 11);
    up_TTO_TH1D hist1D_p_km = std::make_unique<TTO_TH1D>("p_k_minus", "", 200, 0, 11);
    up_TTO_TH1D hist1D_p_matched_pim = std::make_unique<TTO_TH1D>("p_matched_pi_minus", "", 200, 0, 11);
    up_TTO_TH1D hist1D_p_matched_km = std::make_unique<TTO_TH1D>("p_matched_k_minus", "", 200, 0, 11);

    up_TTO_TH1D hist1D_rich_rq_pim = std::make_unique<TTO_TH1D>("rich_rq_pi_minus", "", 200, 0, 1);

    up_TTO_TH2D hist2D_delta_beta_vs_p_matched_pim = std::make_unique<TTO_TH2D>("delta_beta_vs_p_matched_pi_minus", "", 200, 0, 11, 200, -0.005, 0.005);
    up_TTO_TH2D hist2D_delta_beta_vs_p_matched_km = std::make_unique<TTO_TH2D>("delta_beta_vs_p_matched_k_minus", "", 200, 0, 11, 200, -0.005, 0.005);
    up_TTO_TH2D hist2D_delta_beta_vs_p_matched_electron = std::make_unique<TTO_TH2D>("delta_beta_vs_p_matched_electron", "", 200, 0, 11, 200, -0.005, 0.005);
    up_TTO_TH2D hist2D_delta_beta_vs_p_matched_photon = std::make_unique<TTO_TH2D>("delta_beta_vs_p_matched_photon", "", 200, 0, 11, 200, -0.005, 0.005);

    up_TTO_TH2D hist2D_delta_beta_vs_p_reconstructed_pim = std::make_unique<TTO_TH2D>("delta_beta_vs_p_reconstructed_pi_minus", "", 200, 0, 11, 200, -0.005, 0.005);
    up_TTO_TH2D hist2D_delta_beta_vs_p_matched_pim_in_reconstructed_pim = std::make_unique<TTO_TH2D>("delta_beta_vs_p_matched_pi_minus_in_reconstructed_pi_minus", "", 200, 0, 11, 200, -0.005, 0.005);
    up_TTO_TH2D hist2D_delta_beta_vs_p_matched_km_in_reconstructed_pim = std::make_unique<TTO_TH2D>("delta_beta_vs_p_matched_k_minus_in_reconstructed_pi_minus", "", 200, 0, 11, 200, -0.005, 0.005);
    up_TTO_TH2D hist2D_delta_beta_vs_p_matched_electron_in_reconstructed_pim = std::make_unique<TTO_TH2D>("delta_beta_vs_p_matched_electron_in_reconstructed_pi_minus", "", 200, 0, 11, 200, -0.005, 0.005);
    up_TTO_TH2D hist2D_delta_beta_vs_p_matched_photon_in_reconstructed_pim = std::make_unique<TTO_TH2D>("delta_beta_vs_p_matched_photon_in_reconstructed_pi_minus", "", 200, 0, 11, 200, -0.005, 0.005);

    up_TTO_TH2D hist2D_beta_vs_p_matched_pim = std::make_unique<TTO_TH2D>("beta_vs_p_matched_pi_minus", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_km = std::make_unique<TTO_TH2D>("beta_vs_p_matched_k_minus", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_electron = std::make_unique<TTO_TH2D>("beta_vs_p_matched_electron", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_photon = std::make_unique<TTO_TH2D>("beta_vs_p_matched_photon", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_reconstructed_pim = std::make_unique<TTO_TH2D>("beta_vs_p_reconstructed_pim", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_pim_in_reconstructed_pim = std::make_unique<TTO_TH2D>("beta_vs_p_matched_pim_in_reconstructed_pim", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_km_in_reconstructed_pim = std::make_unique<TTO_TH2D>("beta_vs_p_matched_km_in_reconstructed_pim", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_electron_in_reconstructed_pim = std::make_unique<TTO_TH2D>("beta_vs_p_matched_electron_in_reconstructed_pim", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_photon_in_reconstructed_pim = std::make_unique<TTO_TH2D>("beta_vs_p_matched_photon_in_reconstructed_pim", "", 200, 0, 11, 200, 0.9, 1.05);




    up_TTO_TH2D hist2D_MM_vs_p_reconstructed_pim = std::make_unique<TTO_TH2D>("MM_vs_p_reconstructed_pi_minus", "", 200, 0, 11, 200, 0, 2);
    up_TTO_TH2D hist2D_MM_vs_p_pim_in_reconstructed_pim = std::make_unique<TTO_TH2D>("MM_vs_p_pim_in_reconstructed_pi_minus", "", 200, 0, 11, 200, 0, 2);
    up_TTO_TH2D hist2D_MM_vs_p_km_in_reconstructed_pim = std::make_unique<TTO_TH2D>("MM_vs_p_km_in_reconstructed_pi_minus", "", 200, 0, 11, 200, 0, 2);
    up_TTO_TH2D hist2D_MM_vs_p_electron_in_reconstructed_pim = std::make_unique<TTO_TH2D>("MM_vs_p_electron_in_reconstructed_pi_minus", "", 200, 0, 11, 200, 0, 2);
    up_TTO_TH2D hist2D_MM_vs_p_photon_in_reconstructed_pim = std::make_unique<TTO_TH2D>("MM_vs_p_photon_in_reconstructed_pi_minus", "", 200, 0, 11, 200, 0, 2);



};

struct PionPlus {
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_pip = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_pi_plus", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_kp = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_k_plus", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_proton = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_proton", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_positron = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_positron", "", 200, 0, 11, 200, -1, 1);

    up_TTO_TH2D hist2D_delta_t_vs_p_reconstructed_pip = std::make_unique<TTO_TH2D>("delta_t_vs_p_reconstructed_pi_plus", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_pip_in_recon_pip = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_pi_plus_in_recon_pip", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_kp_in_recon_pip = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_k_plus_in_recon_pip", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_proton_in_recon_pip = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_proton_in_recon_pip", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_positron_in_recon_pip = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_positron_in_recon_pip", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_photon_in_recon_pip = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_photon_in_recon_pip", "", 200, 0, 11, 200, -1, 1);
};

struct NegativeTracks {
    up_TTO_TH2D hist2D_delta_t_vs_p_reconstructed_neg_tracks = std::make_unique<TTO_TH2D>("delta_t_vs_p_reconstructed_negative_tracks", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_pim_in_recon_neg_tracks = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_pi_minus_in_recon_negative_tracks", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_km_in_recon_neg_tracks = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_k_minus_in_recon_negative_tracks", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_electron_in_recon_neg_tracks = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_electron_in_recon_negative_tracks", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_matched_photon_in_recon_neg_tracks = std::make_unique<TTO_TH2D>("delta_t_vs_p_matched_photon_in_recon_negative_tracks", "", 200, 0, 11, 200, -1, 1);
    up_TTO_TH2D hist2D_delta_t_vs_p_reconstructed_pos_tracks = std::make_unique<TTO_TH2D>("delta_t_vs_p_reconstructed_positive_tracks", "", 200, 0, 11, 200, -1, 1);

    up_TTO_TH2D hist2D_beta_vs_p_reconstructed_negative_tracks = std::make_unique<TTO_TH2D>("beta_vs_p_reconstructed_negative_tracks", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_pim_in_negative_tracks = std::make_unique<TTO_TH2D>("beta_vs_p_matched_pim_in_negative_tracks", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_km_in_negative_tracks = std::make_unique<TTO_TH2D>("beta_vs_p_matched_km_in_negative_tracks", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_electron_in_negative_tracks = std::make_unique<TTO_TH2D>("beta_vs_p_matched_electron_in_negative_tracks", "", 200, 0, 11, 200, 0.9, 1.05);
    up_TTO_TH2D hist2D_beta_vs_p_matched_photon_in_negative_tracks = std::make_unique<TTO_TH2D>("beta_vs_p_matched_photon_in_negative_tracks", "", 200, 0, 11, 200, 0.9, 1.05);


    up_TTO_TH2D hist2D_MM_vs_p_reconstructed_negative_tracks = std::make_unique<TTO_TH2D>("MM_vs_p_reconstructed_negative_tracks", "", 200, 0, 11, 200, 0, 2);

};

struct RICHHistograms {
    up_TTO_TH1D hist1D_p_pim = std::make_unique<TTO_TH1D>("rich_p_pi_minus", "", 200, 0, 11);
    up_TTO_TH1D hist1D_p_km = std::make_unique<TTO_TH1D>("rich_p_k_minus", "", 200, 0, 11);
    up_TTO_TH1D hist1D_rich_rq_pim = std::make_unique<TTO_TH1D>("rich_rq_pim", "", 200, 0, 1);
    up_TTO_TH1D hist1D_rich_rq_km = std::make_unique<TTO_TH1D>("rich_rq_km", "", 200, 0, 1);
};

struct PiTrack {
    struct Sector {
        // Histograms to extract the DC fiducial cuts for regions 1 and 2
        up_TTO_TH2D hist2D_edge_vs_chi2ndf_region1;
        up_TTO_TH2D hist2D_edge_vs_chi2ndf_region2;
        up_TTO_TH2D hist2D_edge_vs_chi2ndf_region3;

        // Histograms to extract the CAL fiducial cuts
        up_TTO_TH2D hist2D_u_vs_SF;
        up_TTO_TH2D hist2D_v_vs_SF;
        up_TTO_TH2D hist2D_w_vs_SF;
        up_TTO_TH2D hist2D_u_vs_SF_zoom;
        up_TTO_TH2D hist2D_v_vs_SF_zoom;
        up_TTO_TH2D hist2D_w_vs_SF_zoom;
        up_TTO_TH2D hist2D_u_vs_SF_end_zoom;
        up_TTO_TH2D hist2D_v_vs_SF_end_zoom;
        up_TTO_TH2D hist2D_w_vs_SF_end_zoom;
        up_TTO_TH2D hist2D_u_vs_SF_zoom_cut;
        up_TTO_TH2D hist2D_u_vs_SF_end_zoom_cut;


        Sector(int sector, std::string prefix) {
            hist2D_edge_vs_chi2ndf_region1 = make_hist2d(prefix + "edge_vs_chi2ndf_region1_S" + std::to_string(sector), "", 1000, 0, 60, 200, 0, 100);
            hist2D_edge_vs_chi2ndf_region2 = make_hist2d(prefix + "edge_vs_chi2ndf_region2_S" + std::to_string(sector), "", 1000, 0, 60, 200, 0, 100);
            hist2D_edge_vs_chi2ndf_region3 = make_hist2d(prefix + "edge_vs_chi2ndf_region3_S" + std::to_string(sector), "", 1000, 0, 60, 200, 0, 100);

            hist2D_u_vs_SF = make_hist2d(prefix + "electron_u_vs_SF_S" + std::to_string(sector), "", 200, 0, 400, 200, 0, 0.4);
            hist2D_v_vs_SF = make_hist2d(prefix + "electron_v_vs_SF_S" + std::to_string(sector), "", 200, 0, 400, 200, 0, 0.4);
            hist2D_w_vs_SF = make_hist2d(prefix + "electron_w_vs_SF_S" + std::to_string(sector), "", 200, 0, 400, 200, 0, 0.4);
            hist2D_u_vs_SF_zoom = make_hist2d(prefix + "electron_u_vs_SF_zoom_S" + std::to_string(sector), "", 200, 0, 110, 200, 0, 0.4);
            hist2D_v_vs_SF_zoom = make_hist2d(prefix + "electron_v_vs_SF_zoom_S" + std::to_string(sector), "", 200, 0, 70, 200, 0, 0.4);
            hist2D_w_vs_SF_zoom = make_hist2d(prefix + "electron_w_vs_SF_zoom_S" + std::to_string(sector), "", 200, 0, 70, 200, 0, 0.4);
            hist2D_u_vs_SF_end_zoom = make_hist2d(prefix + "electron_u_vs_SF_end_zoom_S" + std::to_string(sector), "", 200, 300, 450, 200, 0, 0.4);
            hist2D_v_vs_SF_end_zoom = make_hist2d(prefix + "electron_v_vs_SF_end_zoom_S" + std::to_string(sector), "", 200, 250, 400, 200, 0, 0.4);
            hist2D_w_vs_SF_end_zoom = make_hist2d(prefix + "electron_w_vs_SF_end_zoom_S" + std::to_string(sector), "", 200, 250, 400, 200, 0, 0.4);
            hist2D_u_vs_SF_zoom_cut = make_hist2d(prefix + "electron_u_vs_SF_zoom_cut_S" + std::to_string(sector), "", 200, 0, 110, 200, 0, 0.4);
            hist2D_u_vs_SF_end_zoom_cut = make_hist2d(prefix + "electron_u_vs_SF_end_zoom_cut_S" + std::to_string(sector), "", 200, 300, 450, 200, 0, 0.4);

        }
    };

    Sector sector_histograms[7];

    PiTrack(std::string prefix)
        : sector_histograms{Sector(1, prefix), Sector(2, prefix), Sector(3, prefix), Sector(4, prefix),
                            Sector(5, prefix), Sector(6, prefix), Sector(0, prefix)} {
    }
};

struct Histograms {
    Electron electron;
    PionMinus pion_minus;
    PionPlus pion_plus;
    NegativeTracks negative_tracks;
    RICHHistograms rich;
    PiTrack pim_track{"pim_track_"};
    PiTrack pip_track{"pip_track_"};
};

}  // namespace study1
