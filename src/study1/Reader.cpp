#include <cmath>
#include <cstdint>
#include <study1/Reader.hpp>
#include <vector>
#include "Core/Banks.hpp"
#include "Core/Constants.hpp"
#include "Core/Cuts.hpp"
#include "Core/PIDs.hpp"
#include "Core/Particle.hpp"
#include "Core/ParticleSelector.hpp"
#include "Core/ReadBank.hpp"
#include "bank.h"
#include "fmt/base.h"
#include "hipoeventiterator.h"

namespace study1 {

Reader::Reader(Histograms& histograms, Core::Target target, Core::Polarity polarity)
    : m_histograms(histograms), m_target(target), m_polarity(polarity) {}

Reader::~Reader() = default;

auto Reader::operator()(hipo::iter_event& event) -> void {
    hipo::bank_map REC_Event = event.get_bank_map("REC::Event");
    if (REC_Event.get_rows() == 0) return;
    double start_time = REC_Event.get_as<double>("startTime", 0);
    hipo::bank_map REC_Particle = event.get_bank_map("REC::Particle");
    hipo::bank_map REC_Scintillator = event.get_bank_map("REC::Scintillator");
    hipo::bank_map REC_Traj = event.get_bank_map("REC::Traj");
    hipo::bank_map REC_Track = event.get_bank_map("REC::Track");
    hipo::bank_map REC_Calorimeter = event.get_bank_map("REC::Calorimeter");

    m_event_count++;

    Core::ParticleMap particles = Core::ParticleSelector::extract_particles(REC_Particle, {Core::PIDs::ELECTRON, Core::PIDs::PIMINUS, Core::PIDs::PIPLUS, Core::PIDs::KMINUS, Core::PIDs::KPLUS});

    // Select trigger electron
    const auto& electrons = particles[Core::PIDs::ELECTRON];
    std::optional<Core::Particle> trigger_e = Core::find_trigger_electron(electrons, {2000, 4000});
    if (!trigger_e.has_value()) return;
    m_nb_trigger_electron++;

    // Apply electron ID
    Core::CalorimeterBank calorimeter = Core::read_Calorimeter_bank(REC_Calorimeter.get_bank(), trigger_e->index());
    Core::DCTrajBank traj = Core::read_Traj_bank(REC_Traj.get_bank(), trigger_e->index());
    Core::TrackBank track = Core::read_Track_bank(REC_Track.get_bank(), trigger_e->index());
    // Core::ElectronCuts::CutResults cuts_results = Core::ElectronCuts::apply_all_cuts(trigger_e.value(), calorimeter, traj, m_target, m_polarity, m_sf_cut);
    // if (!cuts_results.passes_all_required()) return;

    // Apply each electron cut individually for study the effects on the survival rates
    auto electron = trigger_e.value();
    auto target = m_target;
    auto polarity = m_polarity;
    auto& sf_cut = m_sf_cut;
    Core::ElectronCuts::CutResults results;

    // Calculate derived quantities
    const double E_tot = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
    const double SF = E_tot / electron.p();
    const double p_e = electron.p();
    const int sector = calorimeter.pcal.sector;
    const double edge1 = traj.region1.edge;
    const double edge2 = traj.region2.edge;
    const double edge3 = traj.region3.edge;

    // Apply individual cuts
    results.chi2pid = Core::ElectronCuts::pass_chi2pid_cut(electron.chi2pid());
    results.momentum = Core::ElectronCuts::pass_electron_momentum_cut(p_e);
    results.vz = Core::ElectronCuts::pass_electron_vz_cut(electron.vz(), target);
    results.sampling_fraction = Core::ElectronCuts::pass_sampling_fraction_cut(SF, p_e, sector, target, polarity, sf_cut);
    results.pcal_fiducial = Core::ElectronCuts::pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw);
    results.dc_chi2ndf = Core::ElectronCuts::pass_chi2ndf_cut(edge1, edge2, edge3);

    // Apply electron cuts sequentially and use counters to track survival rates
    if (results.chi2pid == true) {
        if (results.momentum == true) {
            m_mom_cut_survivors++;
            if (results.sampling_fraction == true) {
                m_sf_cut_survivors++;
                if (results.pcal_fiducial == true) {
                    m_pcal_cut_survivors++;
                    if (results.dc_chi2ndf == true) {
                        m_dc_cut_survivors++;
                        if (results.vz == true) {
                            m_vz_cut_survivors++;
                        }
                    }
                }
            }
        }
    }

    // Check lu vs lw and lu and lw plots to find issues
    m_histograms.electron.hist2D_lu_vs_lv_e_pcal->Get()->Fill(calorimeter.pcal.lu, calorimeter.pcal.lv);
    m_histograms.electron.hist2D_lu_vs_lw_e_pcal->Get()->Fill(calorimeter.pcal.lu, calorimeter.pcal.lw);
    m_histograms.electron.hist2D_lu_vs_lv_e_in->Get()->Fill(calorimeter.inner.lu, calorimeter.inner.lv);
    m_histograms.electron.hist2D_lu_vs_lw_e_in->Get()->Fill(calorimeter.inner.lu, calorimeter.inner.lw);
    m_histograms.electron.hist2D_lu_vs_lv_e_out->Get()->Fill(calorimeter.outer.lu, calorimeter.outer.lv);
    m_histograms.electron.hist2D_lu_vs_lw_e_out->Get()->Fill(calorimeter.outer.lu, calorimeter.outer.lw);

    // Get the chi2/ndf vs edge 2d histogram as function of momentum
    double chi2ndf = track.chi2 / track.ndf;
    double p = electron.p();
    double edge_r1 = traj.region1.edge;
    double edge_r2 = traj.region2.edge;
    double edge_r3 = traj.region3.edge;

    if (1.0 <= p && p < 2.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_1_2->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_1_2->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_1_2->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (2.0 <= p && p < 3.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_2_3->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_2_3->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_2_3->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (3.0 <= p && p < 4.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_3_4->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_3_4->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_3_4->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (4.0 <= p && p < 5.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_4_5->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_4_5->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_4_5->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (5.0 <= p && p < 6.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_5_6->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_5_6->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_5_6->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (6.0 <= p && p < 7.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_6_7->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_6_7->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_6_7->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (7.0 <= p && p < 8.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_7_8->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_7_8->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_7_8->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (8.0 <= p && p < 9.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_8_9->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_8_9->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_8_9->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (9.0 <= p && p < 10.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_9_10->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_9_10->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_9_10->Get()->Fill(edge_r3, chi2ndf);
    }


    // Theta binned
    double theta = electron.theta();

    if (5.0 <= theta && theta < 10.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t5_10->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t5_10->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t5_10->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (10.0 <= theta && theta < 15.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t10_15->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t10_15->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t10_15->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (15.0 <= theta && theta < 20.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t15_20->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t15_20->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t15_20->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (20.0 <= theta && theta < 25.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t20_25->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t20_25->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t20_25->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (25.0 <= theta && theta < 30.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t25_30->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t25_30->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t25_30->Get()->Fill(edge_r3, chi2ndf);
    }

    else if (30.0 <= theta && theta < 35.0) {
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r1_t30_35->Get()->Fill(edge_r1, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r2_t30_35->Get()->Fill(edge_r2, chi2ndf);
        m_histograms.electron.hist2D_chi2ndf_vs_edge_r3_t30_35->Get()->Fill(edge_r3, chi2ndf);
    }





}

}  // namespace study1
