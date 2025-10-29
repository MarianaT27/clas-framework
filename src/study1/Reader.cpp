#include <study1/Reader.hpp>
#include "Core/Banks.hpp"
#include "Core/Cuts.hpp"

namespace study1 {

Reader::Reader(Histograms& histograms) : m_histograms(histograms) { }

Reader::~Reader() = default;

auto Reader::operator()(const std::string& file) -> void {
    for (hipo::hipoeventfile events(file); auto& event : events) {
        Core::EventData evt(event, {Core::PIDs::Electron, Core::PIDs::PionMinus});
        if (!evt.has_particles()) continue;

         // Get trigger electron
        std::optional<Core::Particle> trigger_e = evt.trigger_electron({2000, 4000});
        if (!trigger_e.has_value()) continue;

        // Apply electron ID
        Core::CalorimeterBank calorimeter = evt.calorimeter(trigger_e.value());
        Core::DCTrajBank traj = evt.trajectory(trigger_e.value());
        Core::ElectronCuts::CutResults cuts_results = Core::ElectronCuts::apply_all_cuts(trigger_e.value(), calorimeter, traj, m_target, m_polarity, m_sf_cut);
        if (!cuts_results.passes_all_required()) continue;

        
    }
}

}  // namespace study1