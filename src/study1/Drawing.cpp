#include <study1/Drawing.hpp>
#include "Plotting/Draw.hpp"

namespace study1 {

Drawing::Drawing(Histograms& histograms): m_histograms(histograms) {
    m_file = std::make_unique<TFile>("plots/study1_drawings.root", "RECREATE");
}

auto Drawing::draw_electron_kinematics() -> void {
    Plotting::draw_hist1D(m_histograms.electron.hist1D_chi2, {}, {.x_label = "chi2pid_{e}"});
}


}  // namespace study1