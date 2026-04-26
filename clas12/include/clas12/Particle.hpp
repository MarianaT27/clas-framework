#pragma once

// C++ headers
#include <cmath>
#include <limits>

// ROOT headers
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMath.h>

// <fmt> headers
#include <fmt/core.h>
#include <fmt/format.h>

namespace clas12 {

// Forward declarations for the detector-bank structs and the Event the
// per-particle helpers route through. Full definitions live in
// `clas12/bankdefs/generated_readers.hpp` (the bank structs) and
// `clas12/framework/core/Event.hpp` (Event); both are pulled in
// transitively by `clas12/ReadBank.hpp`, which is what callers should
// include if they want `electron.calo()`-style access.
struct CalorimeterBank;
struct DCTrajBank;
struct TrackBank;
struct CherenkovBank;
struct ScintillatorBank;
namespace framework { class Event; }

class Particle {

   private:
    // ****** private members
    int m_pdgcode{0};
    int m_status{0};
    int m_index{0};
    int m_charge{0};

    double m_mass{std::numeric_limits<double>::quiet_NaN()};
    double m_px{std::numeric_limits<double>::quiet_NaN()};
    double m_py{std::numeric_limits<double>::quiet_NaN()};
    double m_pz{std::numeric_limits<double>::quiet_NaN()};
    double m_E{std::numeric_limits<double>::quiet_NaN()};
    double m_vx{std::numeric_limits<double>::quiet_NaN()};
    double m_vy{std::numeric_limits<double>::quiet_NaN()};
    double m_vz{std::numeric_limits<double>::quiet_NaN()};
    double m_vt{std::numeric_limits<double>::quiet_NaN()};
    double m_beta{std::numeric_limits<double>::quiet_NaN()};
    double m_chi2pid{std::numeric_limits<double>::quiet_NaN()};

    // Non-owning back-pointer to the Event this Particle was vended
    // from. nullptr for default-constructed / test-built Particles —
    // calling the detector accessors below on those is undefined.
    clas12::framework::Event* m_event{nullptr};

    // ****** private methods

   public:
    // ****** constructors and destructor
    Particle() = default;
    Particle(int pdg, int status, int index, int charge, double mass, double px, double py, double pz, double E, double vx, double vy, double vz, double vt, double beta, double chi2pid);

    Particle(const Particle&) = default;
    Particle(Particle&&) = default;
    Particle& operator=(const Particle&) = default;
    Particle& operator=(Particle&&) = default;
    ~Particle() = default;

    // ****** public methods
    auto index() const -> int;
    auto charge() const -> int;
    auto pdg() const -> int;
    auto status() const -> int;

    auto PxPyPzEVector() const -> ROOT::Math::PxPyPzEVector;
    auto VxVyVzVtVector() const -> ROOT::Math::XYZTVector;
    auto PxPyPzVector() const -> ROOT::Math::XYZVector;
    auto VxVyVzVector() const -> ROOT::Math::XYZVector;

    auto phi() const -> double;
    auto theta() const -> double;
    auto theta(const Particle& other) const -> double;
    auto p() const -> double;
    auto pt() const -> double;
    auto r() const -> double;
    auto rho() const -> double;
    auto beta() const -> double;
    auto chi2pid() const -> double;
    auto E() const -> double;
    auto vx() const -> double;
    auto vy() const -> double;
    auto vz() const -> double;
    auto vt() const -> double;
    auto px() const -> double;
    auto py() const -> double;
    auto pz() const -> double;
    auto mass() const -> double;

    auto is_valid() const -> bool {
        return !std::isnan(m_px) && !std::isnan(m_py) && !std::isnan(m_pz) && !std::isnan(m_E) && !std::isnan(m_vx) && !std::isnan(m_vy) && !std::isnan(m_vz) && !std::isnan(m_vt);
    }

    /// Set the back-pointer to the Event that vended this Particle.
    /// Called by `Event::load_particles` after construction; not
    /// useful from user code. nullptr is allowed (and is the default
    /// for default-constructed Particles).
    auto set_event(clas12::framework::Event* event) noexcept -> void { m_event = event; }

    /// Non-owning Event pointer; nullptr if this Particle was not
    /// vended from an Event (e.g. test fixtures). Detector accessors
    /// below assume non-null.
    auto event() const noexcept -> clas12::framework::Event* { return m_event; }

    // ============================================================
    // Detector data attached to this particle's pindex.
    // Each forwards once to the matching `read_X_bank(event, index)`
    // free function generated under clas12/bankdefs/. The PindexLinker
    // for the bank is built once per event by the first caller and
    // cached on the Event, so subsequent calls hit the cache.
    //
    // No internal caching: calling `calo()` twice does the work twice.
    // Capture in a local if you need to read multiple times.
    //
    // Defined out-of-line in Particle.cpp, which pulls in the bank
    // structs and Event so this header stays light.
    // ============================================================
    auto calo()         const -> CalorimeterBank;
    auto traj()         const -> DCTrajBank;
    auto track()        const -> TrackBank;
    auto cherenkov()    const -> CherenkovBank;
    auto scintillator() const -> ScintillatorBank;

    // ****** operators
    auto operator==(const Particle& other) const -> bool { return m_index == other.m_index; }

    auto operator!=(const Particle& other) const -> bool { return m_index != other.m_index; }

    auto operator<(const Particle& other) const -> bool { return m_index < other.m_index; }

    auto operator>(const Particle& other) const -> bool { return m_index > other.m_index; }
};
}  // namespace clas12

namespace fmt {
template <>
class formatter<clas12::Particle> {
   public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context>
    constexpr auto format(clas12::Particle const& foo, Context& ctx) const {
        return format_to(ctx.out(), "(PDG: {}, Index: {}, Status: {}, px: {}, py: {}, pz: {}, vx: {}, vy: {}, vz: {})", foo.pdg(), foo.index(), foo.status(), foo.px(), foo.py(), foo.pz(), foo.vx(), foo.vy(), foo.vz());
    }
};

}  // namespace fmt