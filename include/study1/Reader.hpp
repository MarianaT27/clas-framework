#pragma once

// C++ headers
#include <string>

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
#include <Core/ReadBank.hpp>
#include <Core/PIDs.hpp>
#include <study1/Histograms.hpp>
#include <vector>

namespace study1 {

class Reader {
   private:
    using particles_map_t = std::unordered_map<Core::PIDs, std::vector<Core::Particle>>;

    // ****** private variables
    Histograms& m_histograms;
    std::vector<Core::PIDs> m_pids;

    // ****** private constants
    static constexpr double NaN = std::numeric_limits<double>::quiet_NaN();

    // ****** private methods
    auto select_electron(const Core::Particle& electron, const hipo::bank& REC_Calorimeter, const hipo::bank& REC_Cherenkov) const -> bool;
    auto get_topology(const hipo::bank& REC_Particle) -> particles_map_t;

   public:
    // ****** constructors and destructor
    explicit Reader(Histograms& histograms);
    ~Reader();

    // ****** public methods
    auto operator()(const std::string& file) -> void;
};

}  // namespace study1