#pragma once

// Project headers
#include <Plotting/Draw.hpp>
#include <study1/Histograms.hpp>

namespace study1 {
class Drawing {

   private:
    // ****** private members
    Histograms& m_histograms;
    std::unique_ptr<TFile> m_file;

    // ****** private methods

   public:
    // ****** constructors and destructor
    explicit Drawing(Histograms& histograms);
    ~Drawing() { if (m_file) { m_file->Write(); m_file->Close(); } }

    // ****** public methods
    auto draw_electron_kinematics() -> void;
};

}  // namespace study1