#include <study1/Reader.hpp>
#include "Core/PIDs.hpp"

namespace study1 {

Reader::Reader(Histograms& histograms) : m_histograms(histograms) { }

Reader::~Reader() = default;

auto Reader::operator()(const std::string& file) -> void {
    for (hipo::hipoeventfile events(file); auto& event : events) {

        hipo::bank& REC_Particle = event.get_bank("REC::Particle");
        hipo::bank& REC_Calorimeter = event.get_bank("REC::Calorimeter");
        hipo::bank& REC_Cherenkov = event.get_bank("REC::Cherenkov");
        hipo::bank& REC_Event = event.get_bank("REC::Event");

        if (REC_Particle.getRows() == 0) continue;

        particles_map_t map = get_topology(REC_Particle);

        auto electron = Core::find_trigger_electron(map[Core::PIDs::Electron], {2000, 4000}).value_or(Core::Particle{});
        if (!electron.is_valid()) continue;
    }
}

auto Reader::get_topology(const hipo::bank& REC_Particle) -> Topology {
    Topology topology;

    for (int i = 0; i < REC_Particle.getRows(); i++) {
        const int pid = REC_Particle.get<int>("pid", i);
        const int status = REC_Particle.get<int>("status", i);
        const int charge = REC_Particle.get<int>("charge", i);
        const double px = REC_Particle.get<double>("px", i);
        const double py = REC_Particle.get<double>("py", i);
        const double pz = REC_Particle.get<double>("pz", i);
        const double vx = REC_Particle.get<double>("vx", i);
        const double vy = REC_Particle.get<double>("vy", i);
        const double vz = REC_Particle.get<double>("vz", i);
        const double vt = REC_Particle.get<double>("vt", i);
        const double beta = REC_Particle.get<double>("beta", i);
        const double chi2pid = REC_Particle.get<double>("chi2pid", i);
        const double mass = Core::get_mass(pid);
        const double E = Core::compute_energy(px, py, pz, pid);

        if (pid == static_cast<int>(Core::PIDs::Electron))
            topology.electrons.emplace_back(pid, status, i, charge, mass, px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
    
    }
    return topology;
}

auto Reader::select_electron(const Core::Particle& electron, const hipo::bank& REC_Calorimeter, const hipo::bank& REC_Cherenkov) const -> bool {
    bool pass = false;

    Core::CherenkovBank cherenkov = Core::read_Cherenkov_bank(REC_Cherenkov, electron.index());
    Core::CalorimeterBank calorimeter = Core::read_Calorimeter_bank(REC_Calorimeter, electron.index());

    double ETot = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
    double SF = ETot / electron.p();

    // Cuts ---------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------

    // Get variables for electrons kinematics to apply the cuts -----------------------------------
    const double p_e = electron.p();
    const double chi2_e = electron.chi2pid();
    const double vz_e = electron.vz();
    const double phi_e = electron.phi();
    const double theta_e = electron.theta();
    //---------------------------------------------------------------------------------------------

    // Cuts ---------------------------------------------------------------------------------------
    // const bool cut_chi2 = chi2_min_e < chi2_e && chi2_e < chi2_max_e;
    // const bool cut_vz = vz_min_e <= vz_e && vz_e <= vz_max_e;
    // const bool cuts = cut_chi2 && cut_vz;
    // --------------------------------------------------------------------------------------------

    // Fill the histograms before any cuts are applied --------------------------------------------
    m_histograms.electron.hist1D_p->Get()->Fill(p_e);
    m_histograms.electron.hist1D_phi->Get()->Fill(phi_e);
    m_histograms.electron.hist1D_theta->Get()->Fill(theta_e);
    m_histograms.electron.hist1D_chi2->Get()->Fill(chi2_e);
    m_histograms.electron.hist1D_vz->Get()->Fill(vz_e);

    //---------------------------------------------------------------------------------------------

    // Fill histograms after cuts: ----------------------------------------------------------------
    // if (cut_chi2) m_histograms.electron.hist1D_vz_cut->Get()->Fill(vz_e);
    // if (cut_vz) m_histograms.electron.hist1D_chi2_cut->Get()->Fill(chi2_e);

    // if (cuts) {
    //     m_histograms.electron.hist1D_p_cut->Get()->Fill(p_e);
    //     m_histograms.electron.hist1D_phi_cut->Get()->Fill(phi_e);
    //     m_histograms.electron.hist1D_theta_cut->Get()->Fill(theta_e);
    //     pass = true;
    // }
    //---------------------------------------------------------------------------------------------

    return pass;
}

}  // namespace study1