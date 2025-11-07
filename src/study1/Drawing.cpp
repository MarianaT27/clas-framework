#include <study1/Drawing.hpp>
#include <fstream>
#include "Plotting/Draw.hpp"


namespace study1 {

Drawing::Drawing(Histograms& histograms): m_histograms(histograms) {
    m_file = std::make_unique<TFile>("../plots/pion_contamination.root", "RECREATE");
}

auto Drawing::draw_electron_kinematics() -> void {
    Plotting::draw_hist1D(m_histograms.electron.hist1D_chi2, {}, {.x_label = "chi2pid_{e}"});
}

auto Drawing::draw_pion_minus() -> void {
    Plotting::draw_hist1D(m_histograms.pion_minus.hist1D_p_electron, m_file, {.x_label = "p (GeV/c)"});
    Plotting::draw_hist1D(m_histograms.pion_minus.hist1D_p_pim, m_file, {.x_label = "p (GeV/c)"});
    Plotting::draw_hist1D(m_histograms.pion_minus.hist1D_p_km, m_file, {.x_label = "p (GeV/c)"});

    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_km, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_electron, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_photon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});

    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_pim_in_recon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_km_in_recon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_electron_in_recon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_photon_in_recon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});

    // Cuts applied -------------------------------------------------------------------------------
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_pim_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_km_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_electron_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_photon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});

    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_reconstructed_pim_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_pim_in_recon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_km_in_recon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_electron_in_recon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_matched_photon_in_recon_cut, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    // --------------------------------------------------------------------------------------------

    // RICH matched particles ---------------------------------------------------------------------
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_rich_matched_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_rich_matched_km, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_t_vs_p_rich_matched_electron, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist1D(m_histograms.pion_minus.hist1D_rich_rq_pim, m_file, {.x_label = "RICH RQ", .y_label = "Counts"});

    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_kp, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_proton, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_positron, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});

    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_reconstructed_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_pip_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_kp_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_proton_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_positron_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.pion_plus.hist2D_delta_t_vs_p_matched_photon_in_recon_pip, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});


    // Negative tracks that are not electrons ------------------------------------------------------
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_reconstructed_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_reconstructed_pos_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_matched_pim_in_recon_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_matched_km_in_recon_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_matched_electron_in_recon_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_delta_t_vs_p_matched_photon_in_recon_neg_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta t (ns)"});





    // --------------------------------------------------------------------------------------------
    // Delta beta plots ----------------------------------------------------------------------------
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_km, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_electron, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_photon, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});

    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_pim_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_km_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_electron_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_delta_beta_vs_p_matched_photon_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#Delta #beta"});
    // --------------------------------------------------------------------------------------------

    // Beta vs p plots -----------------------------------------------------------------------------
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_km, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_electron, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_photon, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_pim_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_km_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_electron_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.pion_minus.hist2D_beta_vs_p_matched_photon_in_reconstructed_pim, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_reconstructed_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_matched_pim_in_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_matched_km_in_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_matched_electron_in_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    Plotting::draw_hist2D(m_histograms.negative_tracks.hist2D_beta_vs_p_matched_photon_in_negative_tracks, m_file, {.x_label = "p (GeV/c)", .y_label = "#beta"});
    // --------------------------------------------------------------------------------------------


}

auto Drawing::draw_rich_histograms() -> void {
    Plotting::draw_hist1D(m_histograms.rich.hist1D_p_pim, m_file, {.x_label = "p (GeV/c)"});
    Plotting::draw_hist1D(m_histograms.rich.hist1D_p_km, m_file, {.x_label = "p (GeV/c)"});
    Plotting::draw_hist1D(m_histograms.rich.hist1D_rich_rq_pim, m_file, {.x_label = "RICH RQ pi-"});
    Plotting::draw_hist1D(m_histograms.rich.hist1D_rich_rq_km, m_file, {.x_label = "RICH RQ K-"});
}
}  // namespace study1