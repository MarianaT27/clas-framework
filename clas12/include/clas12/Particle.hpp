#pragma once

// C++ headers
#include <array>
#include <cmath>
#include <limits>

// ROOT headers
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMath.h>

// <fmt> headers
#include <fmt/core.h>
#include <fmt/format.h>


// Forward declarations for the detector-bank structs and the Event the
// per-particle helpers route through. Full definitions live in
// `clas12/bankdefs/generated_readers.hpp` (the bank structs and the
// inline `read_*_bank` accessor functions) and
// `clas12/framework/core/Event.hpp` (Event); both are pulled in
// transitively by `clas12/ReadBank.hpp`, which is what callers should
// include if they want `electron.calo()`-style access.
struct CalorimeterBank;
struct DCTrajBank;
struct TrackBank;
struct CherenkovBank;
struct ScintillatorBank;
namespace framework { class Event; }

// Forward-declare the bank reader free functions so the templated
// detector accessors below can pass two-phase lookup at the point
// Particle.hpp is parsed (which is *before* Event.hpp has finished
// including its dependencies). The full inline definitions live in
// clas12/bankdefs/generated_readers.hpp and must be visible at the
// call site of any detector accessor.
inline auto read_Calorimeter_bank(framework::Event& event, int pindex) -> CalorimeterBank;
inline auto read_Traj_bank        (framework::Event& event, int pindex) -> DCTrajBank;
inline auto read_Track_bank       (framework::Event& event, int pindex) -> TrackBank;
inline auto read_Cherenkov_bank   (framework::Event& event, int pindex) -> CherenkovBank;
inline auto read_Scintillator_bank(framework::Event& event, int pindex) -> ScintillatorBank;

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
    framework::Event* m_event{nullptr};

    // ****** private methods

   public:
    // ****** constructors and destructor
    Particle() = default;
    Particle(int pdg, int status, int index, int charge, double mass,
             double px, double py, double pz, double E,
             double vx, double vy, double vz, double vt,
             double beta, double chi2pid)
        : m_pdgcode(pdg), m_status(status), m_index(index), m_charge(charge),
          m_mass(mass), m_px(px), m_py(py), m_pz(pz), m_E(E),
          m_vx(vx), m_vy(vy), m_vz(vz), m_vt(vt),
          m_beta(beta), m_chi2pid(chi2pid) {}

    Particle(const Particle&) = default;
    Particle(Particle&&) = default;
    Particle& operator=(const Particle&) = default;
    Particle& operator=(Particle&&) = default;
    ~Particle() = default;

    // ****** public methods
    auto index()   const -> int { return m_index; }
    auto charge()  const -> int { return m_charge; }
    auto pdg()     const -> int { return m_pdgcode; }
    auto status()  const -> int { return m_status; }

    auto PxPyPzEVector()   const -> ROOT::Math::PxPyPzEVector { return {m_px, m_py, m_pz, m_E}; }
    auto VxVyVzVtVector()  const -> ROOT::Math::XYZTVector    { return {m_vx, m_vy, m_vz, m_vt}; }
    auto PxPyPzVector()    const -> ROOT::Math::XYZVector     { return {m_px, m_py, m_pz}; }
    auto VxVyVzVector()    const -> ROOT::Math::XYZVector     { return {m_vx, m_vy, m_vz}; }

    auto phi() const -> double {
        return m_px == 0.0 && m_py == 0.0 ? 0.0
                                          : TMath::ATan2(m_py, m_px) * 180.0 / TMath::Pi();
    }
    auto theta() const -> double {
        return (m_pz == 0) ? TMath::PiOver2() * 180.0 / TMath::Pi()
                           : TMath::ACos(m_pz / p()) * 180.0 / TMath::Pi();
    }
    auto theta(const Particle& other) const -> double {
        double product = p() * other.p();
        product = product == 0 ? 1
                               : (m_px * other.px() + m_py * other.py() + m_pz * other.pz()) / product;
        product = std::max(std::min(product, 1.0), -1.0);
        return TMath::ACos(product);
    }
    auto p()       const -> double { return TMath::Sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz); }
    auto pt()      const -> double { return TMath::Sqrt(m_px * m_px + m_py * m_py); }
    auto r()       const -> double { return TMath::Sqrt(m_vx * m_vx + m_vy * m_vy); }
    auto rho()     const -> double { return TMath::Sqrt(m_vx * m_vx + m_vy * m_vy + m_vz * m_vz); }
    auto beta()    const -> double { return m_beta; }
    auto chi2pid() const -> double { return m_chi2pid; }
    auto E()       const -> double { return m_E; }
    auto vx()      const -> double { return m_vx; }
    auto vy()      const -> double { return m_vy; }
    auto vz()      const -> double { return m_vz; }
    auto vt()      const -> double { return m_vt; }
    auto px()      const -> double { return m_px; }
    auto py()      const -> double { return m_py; }
    auto pz()      const -> double { return m_pz; }
    auto mass()    const -> double { return m_mass; }

    auto is_valid() const -> bool {
        return !std::isnan(m_px) && !std::isnan(m_py) && !std::isnan(m_pz) && !std::isnan(m_E) && !std::isnan(m_vx) && !std::isnan(m_vy) && !std::isnan(m_vz) && !std::isnan(m_vt);
    }

    /// Set the back-pointer to the Event that vended this Particle.
    /// Called by `Event::load_particles` after construction; not
    /// useful from user code. nullptr is allowed (and is the default
    /// for default-constructed Particles).
    auto set_event(framework::Event* event) noexcept -> void { m_event = event; }

    /// Non-owning Event pointer; nullptr if this Particle was not
    /// vended from an Event (e.g. test fixtures). Detector accessors
    /// below assume non-null.
    auto event() const noexcept -> framework::Event* { return m_event; }

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
    // Each accessor is a defaulted-template member with a deduced
    // return type so both the body AND the return type are parsed only
    // at the call site (where the user has `clas12/ReadBank.hpp` fully
    // included). This breaks the otherwise-circular include chain
    // Particle.hpp ← Event.hpp ← read_X_bank → bank structs.
    // ============================================================
    // Defaulted template parameter Ev makes the call expression
    // dependent (Ev& binds m_event), so both return-type deduction and
    // body parsing happen at the call site — where the user has
    // <clas12/ReadBank.hpp> included and the bank structs / read_X_bank
    // definitions are complete.
    template <typename Ev = framework::Event>
    auto calo() const {
        Ev& ev = *m_event;
        return read_Calorimeter_bank(ev, m_index);
    }
    template <typename Ev = framework::Event>
    auto traj() const {
        Ev& ev = *m_event;
        return read_Traj_bank(ev, m_index);
    }
    template <typename Ev = framework::Event>
    auto track() const {
        Ev& ev = *m_event;
        return read_Track_bank(ev, m_index);
    }
    template <typename Ev = framework::Event>
    auto cherenkov() const {
        Ev& ev = *m_event;
        return read_Cherenkov_bank(ev, m_index);
    }
    template <typename Ev = framework::Event>
    auto scintillator() const {
        Ev& ev = *m_event;
        return read_Scintillator_bank(ev, m_index);
    }

    /// Three DC region tracks for this particle's trajectory, in order
    /// {region1, region2, region3}. Reads `traj()` once internally,
    /// then `read_Track_bank` for each region's index. The PindexLinker
    /// for REC::Track is cached on the Event after the first call from
    /// any particle, so subsequent particles in the same event reuse
    /// it.
    template <typename Ev = framework::Event>
    auto region_tracks() const {
        Ev& ev = *m_event;
        const auto t = read_Traj_bank(ev, m_index);
        using bank_t = decltype(read_Track_bank(ev, 0));
        return std::array<bank_t, 3>{
            read_Track_bank(ev, t.region1_index),
            read_Track_bank(ev, t.region2_index),
            read_Track_bank(ev, t.region3_index),
        };
    }

    // ****** operators
    auto operator==(const Particle& other) const -> bool { return m_index == other.m_index; }

    auto operator!=(const Particle& other) const -> bool { return m_index != other.m_index; }

    auto operator<(const Particle& other) const -> bool { return m_index < other.m_index; }

    auto operator>(const Particle& other) const -> bool { return m_index > other.m_index; }
};

namespace fmt {
template <>
class formatter<Particle> {
   public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context>
    constexpr auto format(Particle const& foo, Context& ctx) const {
        return format_to(ctx.out(), "(PDG: {}, Index: {}, Status: {}, px: {}, py: {}, pz: {}, vx: {}, vy: {}, vz: {})", foo.pdg(), foo.index(), foo.status(), foo.px(), foo.py(), foo.pz(), foo.vx(), foo.vy(), foo.vz());
    }
};

}  // namespace fmt