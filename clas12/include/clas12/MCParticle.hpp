#pragma once

// C++ headers
#include <cmath>
#include <limits>
#include <vector>

// ROOT headers
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMath.h>

// fmt headers
#include <fmt/core.h>
#include <fmt/format.h>


/**
 * @brief Monte Carlo truth particle for generator-level information
 *
 * MCParticle represents generator-level (truth) information for particles
 * in simulated events. It mirrors the Particle class but is specifically
 * for MC truth data from the MC::Particle bank.
 *
 * This class enables:
 * - Efficiency studies (comparing reconstructed to generated)
 * - Resolution measurements (p_rec - p_true)
 * - Background studies (identifying true signal vs combinatorial)
 * - Decay chain analysis (following parent-daughter relationships)
 */
class MCParticle {

   private:
    // Generator-level particle properties
    int m_pdgcode{0};       ///< PDG particle ID
    int m_index{-1};        ///< Index in MC::Particle bank
    int m_parent_index{-1}; ///< Parent particle index (-1 if primary)

    // Generator-level four-momentum
    double m_px{std::numeric_limits<double>::quiet_NaN()};
    double m_py{std::numeric_limits<double>::quiet_NaN()};
    double m_pz{std::numeric_limits<double>::quiet_NaN()};
    double m_E{std::numeric_limits<double>::quiet_NaN()};
    double m_mass{std::numeric_limits<double>::quiet_NaN()};

    // Generator-level vertex (production point)
    double m_vx{std::numeric_limits<double>::quiet_NaN()};
    double m_vy{std::numeric_limits<double>::quiet_NaN()};
    double m_vz{std::numeric_limits<double>::quiet_NaN()};
    double m_vt{0.0}; ///< Production time (typically 0 for primaries)

    int m_matched_rec_index{-1}; ///< Matched reconstructed particle index (-1 if unmatched)
    float m_match_quality{0.0f}; ///< Match quality from MC::RecMatch bank

   public:
    // ========================================================================
    // Constructors
    // ========================================================================

    MCParticle() = default;

    /**
     * @brief Construct MC particle with full information
     *
     * @param pdg PDG particle ID
     * @param index Index in MC::Particle bank
     * @param parent_index Parent particle index (-1 for primary)
     * @param px X momentum component [GeV/c]
     * @param py Y momentum component [GeV/c]
     * @param pz Z momentum component [GeV/c]
     * @param E Energy [GeV]
     * @param mass Mass [GeV/c^2]
     * @param vx Production vertex x [cm]
     * @param vy Production vertex y [cm]
     * @param vz Production vertex z [cm]
     * @param vt Production time [ns]
     */
    MCParticle(int pdg, int index, int parent_index,
               double px, double py, double pz, double E, double mass,
               double vx, double vy, double vz, double vt = 0.0)
        : m_pdgcode(pdg)
        , m_index(index)
        , m_parent_index(parent_index)
        , m_px(px)
        , m_py(py)
        , m_pz(pz)
        , m_E(E)
        , m_mass(mass)
        , m_vx(vx)
        , m_vy(vy)
        , m_vz(vz)
        , m_vt(vt) {}

    // Default copy/move
    MCParticle(const MCParticle&) = default;
    MCParticle(MCParticle&&) = default;
    MCParticle& operator=(const MCParticle&) = default;
    MCParticle& operator=(MCParticle&&) = default;
    ~MCParticle() = default;

    // ========================================================================
    // Basic Accessors
    // ========================================================================

    /// Get PDG particle ID
    [[nodiscard]] auto pdg() const -> int { return m_pdgcode; }

    /// Get index in MC::Particle bank
    [[nodiscard]] auto index() const -> int { return m_index; }

    /// Get parent particle index (-1 if primary vertex)
    [[nodiscard]] auto parent_index() const -> int { return m_parent_index; }

    /// Get particle mass [GeV/c^2]
    [[nodiscard]] auto mass() const -> double { return m_mass; }

    // ========================================================================
    // Momentum Components
    // ========================================================================

    /// Get x-momentum component [GeV/c]
    [[nodiscard]] auto px() const -> double { return m_px; }

    /// Get y-momentum component [GeV/c]
    [[nodiscard]] auto py() const -> double { return m_py; }

    /// Get z-momentum component [GeV/c]
    [[nodiscard]] auto pz() const -> double { return m_pz; }

    /// Get energy [GeV]
    [[nodiscard]] auto E() const -> double { return m_E; }

    /// Get total momentum magnitude [GeV/c]
    [[nodiscard]] auto p() const -> double {
        return std::sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz);
    }

    /// Get transverse momentum [GeV/c]
    [[nodiscard]] auto pt() const -> double {
        return std::sqrt(m_px * m_px + m_py * m_py);
    }

    /// Get polar angle [degrees]
    [[nodiscard]] auto theta() const -> double {
        return std::atan2(pt(), m_pz) * TMath::RadToDeg();
    }

    /// Get azimuthal angle [degrees]
    [[nodiscard]] auto phi() const -> double {
        return std::atan2(m_py, m_px) * TMath::RadToDeg();
    }

    /// Get rapidity
    [[nodiscard]] auto rapidity() const -> double {
        if (std::abs(m_E - m_pz) < 1e-10) return 0.0;
        return 0.5 * std::log((m_E + m_pz) / (m_E - m_pz));
    }

    /// Get pseudorapidity
    [[nodiscard]] auto eta() const -> double {
        double theta_rad = theta() * TMath::DegToRad();
        if (std::abs(theta_rad) < 1e-10) return 0.0;
        return -std::log(std::tan(theta_rad / 2.0));
    }

    // ========================================================================
    // Vertex Components
    // ========================================================================

    /// Get production vertex x [cm]
    [[nodiscard]] auto vx() const -> double { return m_vx; }

    /// Get production vertex y [cm]
    [[nodiscard]] auto vy() const -> double { return m_vy; }

    /// Get production vertex z [cm]
    [[nodiscard]] auto vz() const -> double { return m_vz; }

    /// Get production time [ns]
    [[nodiscard]] auto vt() const -> double { return m_vt; }

    // ========================================================================
    // ROOT Vector Representations
    // ========================================================================

    /// Get four-momentum as ROOT PxPyPzE vector
    [[nodiscard]] auto PxPyPzEVector() const -> ROOT::Math::PxPyPzEVector {
        return ROOT::Math::PxPyPzEVector(m_px, m_py, m_pz, m_E);
    }

    /// Get three-momentum as ROOT XYZ vector
    [[nodiscard]] auto PxPyPzVector() const -> ROOT::Math::XYZVector {
        return ROOT::Math::XYZVector(m_px, m_py, m_pz);
    }

    /// Get production vertex as ROOT XYZ vector
    [[nodiscard]] auto VxVyVzVector() const -> ROOT::Math::XYZVector {
        return ROOT::Math::XYZVector(m_vx, m_vy, m_vz);
    }

    /// Get production vertex as ROOT XYZT vector
    [[nodiscard]] auto VxVyVzVtVector() const -> ROOT::Math::XYZTVector {
        return ROOT::Math::XYZTVector(m_vx, m_vy, m_vz, m_vt);
    }

    // ========================================================================
    // Generator-Level Status
    // ========================================================================

    /// Check if this is a primary particle (from initial interaction)
    [[nodiscard]] auto is_primary() const -> bool {
        return m_parent_index <= 0;
    }

    /// Check if this is a secondary particle (from decay or interaction)
    [[nodiscard]] auto is_secondary() const -> bool {
        return m_parent_index > 0;
    }

    /// Check if particle has valid momentum
    [[nodiscard]] auto is_valid() const -> bool {
        return !std::isnan(m_px) && !std::isnan(m_py) && !std::isnan(m_pz) && !std::isnan(m_E);
    }

    // ========================================================================
    // MC-Reconstruction Matching
    // ========================================================================

    /// Set matched reconstructed particle index
    auto set_matched_rec_index(int idx) -> void { m_matched_rec_index = idx; }

    /// Get matched reconstructed particle index (-1 if unmatched)
    [[nodiscard]] auto matched_rec_index() const -> int { return m_matched_rec_index; }

    /// Check if this MC particle was reconstructed
    [[nodiscard]] auto is_matched() const -> bool { return m_matched_rec_index >= 0; }

    /// Set match quality (0-1 scale)
    auto set_match_quality(float quality) -> void { m_match_quality = quality; }

    /// Get match quality
    [[nodiscard]] auto match_quality() const -> float { return m_match_quality; }

    // ========================================================================
    // Operators
    // ========================================================================

    auto operator==(const MCParticle& other) const -> bool {
        return m_index == other.m_index;
    }

    auto operator!=(const MCParticle& other) const -> bool {
        return m_index != other.m_index;
    }

    auto operator<(const MCParticle& other) const -> bool {
        return m_index < other.m_index;
    }
};


// ============================================================================
// fmt formatter for MCParticle
// ============================================================================
namespace fmt {
template <>
class formatter<MCParticle> {
   public:
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename Context>
    constexpr auto format(const MCParticle& mc, Context& ctx) const {
        return format_to(ctx.out(),
                         "(MC: PDG={}, idx={}, parent={}, p={:.3f} GeV, theta={:.1f} deg, vz={:.2f} cm, matched={})",
                         mc.pdg(), mc.index(), mc.parent_index(),
                         mc.p(), mc.theta(), mc.vz(),
                         mc.is_matched() ? fmt::format("rec[{}]", mc.matched_rec_index()) : "no");
    }
};
}  // namespace fmt
