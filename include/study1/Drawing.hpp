#pragma once

// Project headers
#include <Plotting/Draw.hpp>
#include <study1/Histograms.hpp>
#include "hipo4/hipoeventiterator.h"

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
    auto draw_pion_minus() -> void;
    auto draw_rich_histograms() -> void;
    auto draw_pion_tracks() -> void;

};

}  // namespace study1