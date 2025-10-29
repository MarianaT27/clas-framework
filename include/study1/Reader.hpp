#pragma once

// C++ headers
#include <map>
#include <string>
#include <vector>

// <fmt> headers
#include <fmt/core.h>

// toml++ headers
#include <toml++/toml.h>

// Project headers
#include <hipo4/hipoeventiterator.h>
#include <hipo4/reader.h>

#include <Core/Constantes.hpp>
#include <Core/Helpers.hpp>
#include <Core/Particle.hpp>
#include <Core/PIDs.hpp>
#include <Core/ParticleSelector.hpp>
#include <Core/ReadBank.hpp>
#include <Core/EventData.hpp>
#include <Core/Cuts.hpp>
#include <Core/SamplingFractionCut.hpp>
#include <Core/Target.hpp>

#include <study1/Histograms.hpp>

namespace study1 {

class Reader {
   private:
    // ****** private constants

    // ****** private variables
    Histograms& m_histograms;
    Core::SamplingFractionCut m_sf_cut;
    Core::Target m_target = Core::Target::LD2;
    Core::Polarity m_polarity = Core::Polarity::Outbending;

    // ****** private methods
    auto select_electron(const Core::Particle& electron, const hipo::bank& REC_Calorimeter, const hipo::bank& REC_Cherenkov) const -> bool;

   public:
    // ****** constructors and destructor
    explicit Reader(Histograms& histograms);
    ~Reader();

    // ****** public methods
    auto operator()(const std::string& file) -> void;
};

}  // namespace study1