#pragma once

// C++ headers
#include <atomic>
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

#include <Core/Constants.hpp>
#include <Core/Helpers.hpp>
#include <Core/Particle.hpp>
#include <Core/PIDs.hpp>
#include <Core/ParticleSelector.hpp>
#include <Core/ReadBank.hpp>
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
    Core::Target m_target;
    Core::Polarity m_polarity;
    std::atomic_int64_t m_event_count{0};
    std::atomic_int64_t m_nb_trigger_electron{0};
    std::atomic_int64_t m_mom_cut_survivors{0};
    std::atomic_int64_t m_sf_cut_survivors{0};
    std::atomic_int64_t m_pcal_cut_survivors{0};
    std::atomic_int64_t m_dc_cut_survivors{0};
    std::atomic_int64_t m_vz_cut_survivors{0};
    static constexpr double SECTOR_ANGLE = TMath::Pi() / 3.0;

    // ****** private methods
    auto select_electron(const Core::Particle& electron, const hipo::bank& REC_Calorimeter, const hipo::bank& REC_Cherenkov) const -> bool;
    double third_order_poly(double x, double a, double b, double c, double d) const;
    bool cut_delta_t(double p, double delta_t) const;
    auto compute_local_theta_DC(const double x, const double y, const double z, const int sector) const -> double;
    auto fill_mc_histograms(hipo::bank_map &MC_Particle, hipo::bank_map &MC_GenMatch, hipo::bank_map &REC_Particle, hipo::bank_map &REC_Scintillator, hipo::bank_map &REC_Calorimeter, double start_time) -> void;

   public:
    // ****** constructors and destructor
    explicit Reader(Histograms& histograms, Core::Target target = Core::Target::LD2, Core::Polarity polarity = Core::Polarity::Outbending);
    ~Reader();

    // ****** public methods
    auto operator()(hipo::iter_event& event) -> void;
    auto get_event_count() const -> int64_t { return m_event_count.load(); }
    auto get_nb_trigger_electron() const -> int64_t { return m_nb_trigger_electron.load(); }
    auto get_mom_cut_survivors() const -> int64_t { return m_mom_cut_survivors.load(); }
    auto get_sf_cut_survivors() const -> int64_t { return m_sf_cut_survivors.load(); }
    auto get_pcal_cut_survivors() const -> int64_t { return m_pcal_cut_survivors.load(); }
    auto get_dc_cut_survivors() const -> int64_t { return m_dc_cut_survivors.load(); }
    auto get_vz_cut_survivors() const -> int64_t { return m_vz_cut_survivors.load(); }
};

}  // namespace study1