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
#include "hipoeventiterator.h"

namespace study1 {

Reader::Reader(Histograms& histograms)
    : m_histograms(histograms) {}

Reader::~Reader() = default;

auto Reader::operator()(const std::string& file) -> void {
    hipo::hipoeventfile events(file);

    for ( auto event : events) {
        m_event_count++;
        if (m_event_count.load() > 100000000) break;

        hipo::bank_map REC_Event = event.get_bank_map("REC::Event");
        if (REC_Event.get_rows() == 0) continue;
        double start_time = REC_Event.get_as<double>("startTime", 0);

        hipo::bank_map MC_Particle = event.get_bank_map("MC::Particle");
        hipo::bank_map MC_GenMatch = event.get_bank_map("MC::GenMatch");
        hipo::bank_map MC_RecMatch = event.get_bank_map("MC::RecMatch");
        hipo::bank_map REC_Particle = event.get_bank_map("REC::Particle");
        hipo::bank_map REC_Scintillator = event.get_bank_map("REC::Scintillator");
        hipo::bank_map REC_Traj = event.get_bank_map("REC::Traj");
        hipo::bank_map REC_Calorimeter = event.get_bank_map("REC::Calorimeter");

        Core::ParticleMap particles = Core::ParticleSelector::extract_particles(REC_Particle, {Core::PIDs::Electron, Core::PIDs::PionMinus});

        // Select trigger electron
        const auto& electrons = particles[Core::PIDs::Electron];
        std::optional<Core::Particle> trigger_e = Core::find_trigger_electron(electrons, {2000, 4000});
        if (!trigger_e.has_value()) continue;

        // Apply electron ID
        Core::CalorimeterBank calorimeter = Core::read_Calorimeter_bank(REC_Calorimeter.get_bank(), trigger_e->index());
        Core::DCTrajBank traj = Core::read_Traj_bank(REC_Traj.get_bank(), trigger_e->index());
        Core::ElectronCuts::CutResults cuts_results = Core::ElectronCuts::apply_all_cuts(trigger_e.value(), calorimeter, traj, m_target, m_polarity, m_sf_cut);
        if (!cuts_results.passes_all_required()) continue;

        std::vector<Core::Particle> pims = particles[Core::PIDs::PionMinus];

        // negative tracks that are not electrons 
        std::vector<Core::Particle> negative_tracks;
        for (int i = 0; i < REC_Particle.get_rows(); ++i) {
            const int pid = REC_Particle.get_as<int>("pid", i);
            const int status = REC_Particle.get_as<int>("status", i);
            const int charge = REC_Particle.get_as<int>("charge", i);
            const double px = REC_Particle.get_as<double>("px", i);
            const double py = REC_Particle.get_as<double>("py", i);
            const double pz = REC_Particle.get_as<double>("pz", i);
            const double vx = REC_Particle.get_as<double>("vx", i);
            const double vy = REC_Particle.get_as<double>("vy", i);
            const double vz = REC_Particle.get_as<double>("vz", i);
            const double vt = REC_Particle.get_as<double>("vt", i);
            const double beta = REC_Particle.get_as<double>("beta", i);
            const double chi2pid = REC_Particle.get_as<double>("chi2pid", i);

            const double mass = Core::get_mass(pid);
            const double E = Core::compute_energy(px, py, pz, pid);

            Core::Particle particle(pid, status, i, charge, mass, px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
            if (charge < 0 && pid != 11) negative_tracks.push_back(particle);
        }

        for (const auto& pim : pims) {
            Core::ScintillatorBank scintillator = Core::read_Scintillator_bank(REC_Scintillator.get_bank(), pim.index());
            double measured_time = scintillator.time - start_time;
            double l_ftof = scintillator.path;
            double p = pim.p();
            double m = Core::Constants::PionMass;
            double beta_expected = p / std::sqrt(p * p + m * m);
            double t_expected = l_ftof / (beta_expected * Core::Constants::SPEED_OF_LIGHT);
            double delta_t = measured_time - t_expected;
            
            m_histograms.pion_minus.hist2D_delta_t_vs_p_reconstructed_pim->Get()->Fill(p, delta_t);
        }

        for (const auto& track : negative_tracks) {
            Core::ScintillatorBank scintillator = Core::read_Scintillator_bank(REC_Scintillator.get_bank(), track.index());
            double measured_time = scintillator.time - start_time;
            double l_ftof = scintillator.path;
            double p = track.p();
            double m = Core::Constants::PionMass;
            double beta_expected = p / std::sqrt(p * p + m * m);
            double t_expected = l_ftof / (beta_expected * Core::Constants::SPEED_OF_LIGHT);
            double delta_t = measured_time - t_expected;

            m_histograms.negative_tracks.hist2D_delta_t_vs_p_reconstructed_neg_tracks->Get()->Fill(p, delta_t);
        }

    }
}

double Reader::third_order_poly(double x, double a, double b, double c, double d) const {
    return a + b * x + c * x * x + d * x * x * x;
}

bool Reader::cut_delta_t(double p, double delta_t) const {
    const double popt_mean[4] = {-0.02872196, 0.00598219, 0.00158193, -0.00028493};
    const double popt_sigma[4] = {0.25132972, -0.09788015, 0.01734634, -0.00103597};

    double cut_low = third_order_poly(p, popt_mean[0], popt_mean[1], popt_mean[2], popt_mean[3]) -
                     3 * third_order_poly(p, popt_sigma[0], popt_sigma[1], popt_sigma[2], popt_sigma[3]);
    double cut_high = third_order_poly(p, popt_mean[0], popt_mean[1], popt_mean[2], popt_mean[3]) +
                      3 * third_order_poly(p, popt_sigma[0], popt_sigma[1], popt_sigma[2], popt_sigma[3]);

    return cut_low < delta_t && delta_t < cut_high;
}

}  // namespace study1