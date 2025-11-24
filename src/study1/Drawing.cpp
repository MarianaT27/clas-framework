#include <study1/Drawing.hpp>
#include <fstream>
#include "Plotting/Draw.hpp"


namespace study1 {

Drawing::Drawing(Histograms& histograms): m_histograms(histograms) {
    m_file = std::make_unique<TFile>("../plots/electron_dc_fiducial.root", "RECREATE");
}

auto Drawing::draw_electron_kinematics() -> void {
    // Plotting::draw_hist1D(m_histograms.electron.hist1D_chi2, {}, {.x_label = "chi2pid_{e}"});

    Plotting::draw_hist2D(m_histograms.electron.hist2D_lu_vs_lv_e_pcal, m_file, {.x_label = "lu (cm)", .y_label = "lv (cm)"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_lu_vs_lw_e_pcal, m_file, {.x_label = "lu (cm)", .y_label = "lw (cm)"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_lu_vs_lv_e_in, m_file, {.x_label = "lu (cm)", .y_label = "lv (cm)"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_lu_vs_lw_e_in, m_file, {.x_label = "lu (cm)", .y_label = "lw (cm)"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_lu_vs_lv_e_out, m_file, {.x_label = "lu (cm)", .y_label = "lv (cm)"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_lu_vs_lw_e_out, m_file, {.x_label = "lu (cm)", .y_label = "lw (cm)"});

    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_1_2, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_2_3, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_3_4, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_4_5, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_5_6, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_6_7, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_7_8, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_8_9, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_9_10, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});

    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_1_2, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_2_3, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_3_4, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_4_5, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_5_6, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_6_7, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_7_8, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_8_9, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_9_10, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});

    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_1_2, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_2_3, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_3_4, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_4_5, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_5_6, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_6_7, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_7_8, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_8_9, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_9_10, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});


    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t5_10, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t10_15, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t15_20, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t20_25, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t25_30, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t30_35, m_file, {.x_label = "Edge R1 (cm)", .y_label = "#chi^{2}/ndf"});

    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t5_10, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t10_15, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t15_20, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t20_25, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t25_30, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t30_35, m_file, {.x_label = "Edge R2 (cm)", .y_label = "#chi^{2}/ndf"});

    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t5_10, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t10_15, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t15_20, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t20_25, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t25_30, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
    Plotting::draw_hist2D(m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t30_35, m_file, {.x_label = "Edge R3 (cm)", .y_label = "#chi^{2}/ndf"});
}

auto  Drawing::draw_pion_minus() -> void {
    // Plotting::draw_hist1D(m_histograms.pion_minus.hist1D_p_electron, m_file, {.x_label = "p (GeV/c)"});
    // Plotting::draw_hist1D(m_histograms.pion_minus.hist1D_p_pim, m_file, {.x_label = "p (GeV/c)"});
    // Plotting::draw_hist1D(m_histograms.pion_minus.hist1D_p_km, m_file, {.x_label = "p (GeV/c)"});
//
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_km, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_electron, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_photon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
//
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_pim_in_recon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_km_in_recon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_electron_in_recon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_photon_in_recon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
//
    // // Cuts applied -------------------------------------------------------------------------------
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_pim_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_km_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_electron_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_photon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
//
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_reconstructed_pim_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_pim_in_recon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_km_in_recon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_electron_in_recon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_photon_in_recon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // // --------------------------------------------------------------------------------------------
//
    // // RICH matched particles ---------------------------------------------------------------------
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_rich_matched_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_rich_matched_km, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_rich_matched_electron, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist1D(m_histograms.pion_minus.hist1D_rich_rq_pim, m_file, {.x_label = "RICH RQ", .y_label = "Counts"});
//
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_kp, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_proton, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_positron, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
//
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_reconstructed_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_pip_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_kp_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_proton_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_positron_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_photon_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
//
//
    // // Negative tracks that are not electrons ------------------------------------------------------
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_reconstructed_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_reconstructed_pos_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    //
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_matched_pim_in_recon_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_matched_km_in_recon_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_matched_electron_in_recon_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_matched_photon_in_recon_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
//
//
//
//
//
    // // --------------------------------------------------------------------------------------------
    // // Delta beta plots ----------------------------------------------------------------------------
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_km, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_electron, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_photon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
//
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_pim_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_km_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_electron_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_photon_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    // // --------------------------------------------------------------------------------------------
//
    // // Beta vs p plots -----------------------------------------------------------------------------
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_km, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_electron, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_photon, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_pim_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_km_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_electron_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_photon_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});

    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_reconstructed_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_matched_pim_in_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_matched_km_in_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_matched_electron_in_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_matched_photon_in_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    // --------------------------------------------------------------------------------------------


    // Missing Mass vs p plots -------------------------------------------------------------------
    // Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_MM_vs_p_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "Missing Mass (GeV/c^{2})"});
    // Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_MM_vs_p_reconstructed_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "Missing Mass (GeV/c^{2})"});
    // --------------------------------------------------------------------------------------------

}

auto Drawing::draw_rich_histograms() -> void {
    Plotting::draw_hist1D(m_histograms.rich.hist1D_p_pim, m_file, {.x_label = "p (GeV/c)"});
    Plotting::draw_hist1D(m_histograms.rich.hist1D_p_km, m_file, {.x_label = "p (GeV/c)"});
    Plotting::draw_hist1D(m_histograms.rich.hist1D_rich_rq_pim, m_file, {.x_label = "RICH RQ pi-"});
    Plotting::draw_hist1D(m_histograms.rich.hist1D_rich_rq_km, m_file, {.x_label = "RICH RQ K-"});
}


auto Drawing::draw_pion_tracks() -> void {
    for (int sector = 1; sector <= 7; ++sector) {
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_edge_vs_chi2ndf_region1, m_file, {.x_label = "Edge Distance (cm)", .y_label = "#chi^{2}/ndf"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_edge_vs_chi2ndf_region2, m_file, {.x_label = "Edge Distance (cm)", .y_label = "#chi^{2}/ndf"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_edge_vs_chi2ndf_region3, m_file, {.x_label = "Edge Distance (cm)", .y_label = "#chi^{2}/ndf"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_u_vs_SF, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_v_vs_SF, m_file, {.x_label = "v (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_w_vs_SF, m_file, {.x_label = "w (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_u_vs_SF_zoom, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_v_vs_SF_zoom, m_file, {.x_label = "v (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_w_vs_SF_zoom, m_file, {.x_label = "w (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_u_vs_SF_end_zoom, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_v_vs_SF_end_zoom, m_file, {.x_label = "v (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_w_vs_SF_end_zoom, m_file, {.x_label = "w (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_u_vs_SF_zoom_cut, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pim_track.sector_histograms[sector - 1].hist2D_u_vs_SF_end_zoom_cut, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
    }

    for (int sector = 1; sector <= 7; ++sector) {
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_edge_vs_chi2ndf_region1, m_file, {.x_label = "Edge Distance (cm)", .y_label = "#chi^{2}/ndf"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_edge_vs_chi2ndf_region2, m_file, {.x_label = "Edge Distance (cm)", .y_label = "#chi^{2}/ndf"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_edge_vs_chi2ndf_region3, m_file, {.x_label = "Edge Distance (cm)", .y_label = "#chi^{2}/ndf"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_u_vs_SF, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_v_vs_SF, m_file, {.x_label = "v (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_w_vs_SF, m_file, {.x_label = "w (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_u_vs_SF_zoom, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_v_vs_SF_zoom, m_file, {.x_label = "v (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_w_vs_SF_zoom, m_file, {.x_label = "w (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_u_vs_SF_end_zoom, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_v_vs_SF_end_zoom, m_file, {.x_label = "v (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_w_vs_SF_end_zoom, m_file, {.x_label = "w (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_u_vs_SF_zoom_cut, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
        Plotting::draw_hist2D(m_histograms.pip_track.sector_histograms[sector - 1].hist2D_u_vs_SF_end_zoom_cut, m_file, {.x_label = "u (cm)", .y_label = "Sampling Fraction"});
    }
}


}  // namespace study1
