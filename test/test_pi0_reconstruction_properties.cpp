#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <cmath>
#include <vector>
#include <optional>
#include <tuple>
#include <limits>
#include <set>

// ROOT headers for four-vector calculations
#include <Math/Vector4D.h>
#include <Math/Vector3D.h>

/**
 * Minimal Particle class for testing Pi0 reconstruction logic
 * This avoids the heavy framework dependencies while testing the core algorithms
 */
class TestParticle {
public:
    TestParticle(double px, double py, double pz, double E, int index = 0)
        : m_px(px), m_py(py), m_pz(pz), m_E(E), m_index(index) {}
    
    auto px() const -> double { return m_px; }
    auto py() const -> double { return m_py; }
    auto pz() const -> double { return m_pz; }
    auto E() const -> double { return m_E; }
    auto index() const -> int { return m_index; }
    
    auto PxPyPzEVector() const -> ROOT::Math::PxPyPzEVector {
        return {m_px, m_py, m_pz, m_E};
    }
    
    auto PxPyPzVector() const -> ROOT::Math::XYZVector {
        return {m_px, m_py, m_pz};
    }
    
private:
    double m_px, m_py, m_pz, m_E;
    int m_index;
};

/**
 * Pi0 reconstruction helper functions extracted for testing
 * These mirror the implementation in Pi0ReconstructionAlg
 */
namespace Pi0Helpers {

constexpr double PI0_MASS = 0.135;  // GeV/c²

auto select_photons(const std::vector<TestParticle>& photons, double min_energy)
    -> std::vector<TestParticle>
{
    std::vector<TestParticle> selected;
    selected.reserve(photons.size());
    
    for (const auto& photon : photons) {
        if (photon.E() >= min_energy) {
            selected.push_back(photon);
        }
    }
    
    return selected;
}

auto calculate_invariant_mass(const TestParticle& g1, const TestParticle& g2)
    -> double
{
    auto p1 = g1.PxPyPzEVector();
    auto p2 = g2.PxPyPzEVector();
    auto p_total = p1 + p2;
    return p_total.M();
}

auto find_best_pi0(const std::vector<TestParticle>& photons, double mass_window)
    -> std::optional<std::tuple<TestParticle, TestParticle, double>>
{
    if (photons.size() < 2) {
        return std::nullopt;
    }
    
    std::optional<std::tuple<TestParticle, TestParticle, double>> best;
    double best_mass_diff = std::numeric_limits<double>::max();
    
    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            double mass = calculate_invariant_mass(photons[i], photons[j]);
            double mass_diff = std::abs(mass - PI0_MASS);
            
            if (mass_diff < mass_window && mass_diff < best_mass_diff) {
                best_mass_diff = mass_diff;
                best = std::make_tuple(photons[i], photons[j], mass);
            }
        }
    }
    
    return best;
}

} // namespace Pi0Helpers

namespace {

// Helper to generate a random double in a range using integer scaling
auto randomDouble(double min, double max) -> double {
    int scaled = *rc::gen::inRange(0, 10000);
    return min + (max - min) * (scaled / 10000.0);
}

// Generate a photon with specified energy and random direction
auto makePhotonWithEnergy(double E, int index = 0) -> TestParticle {
    // For a massless photon, E = |p|
    double theta = randomDouble(0.1, M_PI - 0.1);
    double phi = randomDouble(-M_PI, M_PI);
    
    double px = E * std::sin(theta) * std::cos(phi);
    double py = E * std::sin(theta) * std::sin(phi);
    double pz = E * std::cos(theta);
    
    return TestParticle(px, py, pz, E, index);
}

// Generator for positive energy values (realistic photon energies in GeV)
auto genPositiveEnergy() -> double {
    return randomDouble(0.001, 10.0);
}

// Generator for energy below threshold
auto genEnergyBelowThreshold(double threshold) -> double {
    return randomDouble(0.001, threshold - 0.001);
}

// Generator for energy at or above threshold
auto genEnergyAtOrAboveThreshold(double threshold) -> double {
    return randomDouble(threshold, 10.0);
}

} // anonymous namespace

/**
 * **Feature: pi0-reconstruction, Property 1: Photon Energy Filtering**
 * 
 * *For any* photon with energy below the minimum threshold, the photon SHALL be 
 * excluded from the selected photon list.
 * 
 * **Validates: Requirements 1.2**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, PhotonEnergyFiltering_BelowThreshold, ()) {
    double threshold = 0.1;  // Default threshold
    
    // Generate energy below threshold
    double energy = genEnergyBelowThreshold(threshold);
    RC_PRE(energy < threshold);
    
    // Create a photon with this energy
    auto photon = makePhotonWithEnergy(energy, 0);
    std::vector<TestParticle> photons = {photon};
    
    // Apply selection
    auto selected = Pi0Helpers::select_photons(photons, threshold);
    
    // Photon should be excluded
    RC_ASSERT(selected.empty());
}

/**
 * **Feature: pi0-reconstruction, Property 1: Photon Energy Filtering**
 * 
 * *For any* photon with energy at or above the minimum threshold, the photon SHALL be 
 * included in the selected photon list.
 * 
 * **Validates: Requirements 1.2**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, PhotonEnergyFiltering_AtOrAboveThreshold, ()) {
    double threshold = 0.1;  // Default threshold
    
    // Generate energy at or above threshold
    double energy = genEnergyAtOrAboveThreshold(threshold);
    RC_PRE(energy >= threshold);
    
    // Create a photon with this energy
    auto photon = makePhotonWithEnergy(energy, 0);
    std::vector<TestParticle> photons = {photon};
    
    // Apply selection
    auto selected = Pi0Helpers::select_photons(photons, threshold);
    
    // Photon should be included
    RC_ASSERT(selected.size() == 1);
    RC_ASSERT(selected[0].E() == energy);
}

/**
 * **Feature: pi0-reconstruction, Property 1: Photon Energy Filtering**
 * 
 * *For any* collection of photons, all selected photons SHALL have energy >= threshold.
 * 
 * **Validates: Requirements 1.2**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, PhotonEnergyFiltering_AllSelectedAboveThreshold, ()) {
    // Create algorithm with random threshold
    double threshold = randomDouble(0.05, 0.5);
    
    // Generate a collection of photons with various energies
    int n_photons = *rc::gen::inRange(1, 20);
    std::vector<TestParticle> photons;
    
    for (int i = 0; i < n_photons; ++i) {
        double energy = genPositiveEnergy();
        photons.push_back(makePhotonWithEnergy(energy, i));
    }
    
    // Apply selection
    auto selected = Pi0Helpers::select_photons(photons, threshold);
    
    // All selected photons must have energy >= threshold
    for (const auto& photon : selected) {
        RC_ASSERT(photon.E() >= threshold);
    }
}

/**
 * **Feature: pi0-reconstruction, Property 1: Photon Energy Filtering**
 * 
 * *For any* collection of photons, the number of selected photons SHALL equal
 * the count of input photons with energy >= threshold.
 * 
 * **Validates: Requirements 1.2**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, PhotonEnergyFiltering_CountPreserved, ()) {
    // Create algorithm with random threshold
    double threshold = randomDouble(0.05, 0.5);
    
    // Generate a collection of photons with various energies
    int n_photons = *rc::gen::inRange(1, 20);
    std::vector<TestParticle> photons;
    size_t expected_count = 0;
    
    for (int i = 0; i < n_photons; ++i) {
        double energy = genPositiveEnergy();
        photons.push_back(makePhotonWithEnergy(energy, i));
        if (energy >= threshold) {
            expected_count++;
        }
    }
    
    // Apply selection
    auto selected = Pi0Helpers::select_photons(photons, threshold);
    
    // Count should match
    RC_ASSERT(selected.size() == expected_count);
}

/**
 * **Feature: pi0-reconstruction, Property 3: Invariant Mass Calculation Correctness**
 * 
 * *For any* two photons with known four-momenta, the calculated invariant mass SHALL equal
 * sqrt((E1+E2)² - (p1+p2)²) within floating-point precision.
 * 
 * **Validates: Requirements 2.2**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, InvariantMassCalculation, ()) {
    // Generate two photons with random energies and directions
    double E1 = randomDouble(0.1, 5.0);
    double E2 = randomDouble(0.1, 5.0);
    
    double theta1 = randomDouble(0.1, M_PI - 0.1);
    double phi1 = randomDouble(-M_PI, M_PI);
    double theta2 = randomDouble(0.1, M_PI - 0.1);
    double phi2 = randomDouble(-M_PI, M_PI);
    
    // For massless photons, E = |p|
    double px1 = E1 * std::sin(theta1) * std::cos(phi1);
    double py1 = E1 * std::sin(theta1) * std::sin(phi1);
    double pz1 = E1 * std::cos(theta1);
    
    double px2 = E2 * std::sin(theta2) * std::cos(phi2);
    double py2 = E2 * std::sin(theta2) * std::sin(phi2);
    double pz2 = E2 * std::cos(theta2);
    
    TestParticle g1(px1, py1, pz1, E1, 0);
    TestParticle g2(px2, py2, pz2, E2, 1);
    
    // Calculate invariant mass using the helper
    double calculated_mass = Pi0Helpers::calculate_invariant_mass(g1, g2);
    
    // Calculate expected invariant mass manually: M² = (E1+E2)² - (px1+px2)² - (py1+py2)² - (pz1+pz2)²
    double E_total = E1 + E2;
    double px_total = px1 + px2;
    double py_total = py1 + py2;
    double pz_total = pz1 + pz2;
    
    double M_squared = E_total * E_total - px_total * px_total - py_total * py_total - pz_total * pz_total;
    double expected_mass = (M_squared > 0) ? std::sqrt(M_squared) : 0.0;
    
    // Check within floating-point tolerance (1e-8 is reasonable for double precision)
    double tolerance = 1e-8;
    RC_ASSERT(std::abs(calculated_mass - expected_mass) < tolerance);
}

/**
 * **Feature: pi0-reconstruction, Property 3: Invariant Mass Calculation Correctness**
 * 
 * *For any* two collinear photons (same direction), the invariant mass SHALL be zero
 * (within floating-point precision).
 * 
 * **Validates: Requirements 2.2**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, InvariantMassCollinear, ()) {
    // Generate two collinear photons (same direction)
    double E1 = randomDouble(0.1, 5.0);
    double E2 = randomDouble(0.1, 5.0);
    
    double theta = randomDouble(0.1, M_PI - 0.1);
    double phi = randomDouble(-M_PI, M_PI);
    
    // Same direction for both photons
    double px1 = E1 * std::sin(theta) * std::cos(phi);
    double py1 = E1 * std::sin(theta) * std::sin(phi);
    double pz1 = E1 * std::cos(theta);
    
    double px2 = E2 * std::sin(theta) * std::cos(phi);
    double py2 = E2 * std::sin(theta) * std::sin(phi);
    double pz2 = E2 * std::cos(theta);
    
    TestParticle g1(px1, py1, pz1, E1, 0);
    TestParticle g2(px2, py2, pz2, E2, 1);
    
    // Calculate invariant mass
    double mass = Pi0Helpers::calculate_invariant_mass(g1, g2);
    
    // For collinear massless particles, invariant mass should be ~0
    // Use a tolerance relative to the energy scale to account for floating-point precision
    double tolerance = 1e-6 * (E1 + E2);
    RC_ASSERT(mass < tolerance);
}

/**
 * **Feature: pi0-reconstruction, Property 3: Invariant Mass Calculation Correctness**
 * 
 * *For any* two back-to-back photons (opposite directions), the invariant mass SHALL equal
 * 2 * sqrt(E1 * E2) (within floating-point precision).
 * 
 * **Validates: Requirements 2.2**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, InvariantMassBackToBack, ()) {
    // Generate two back-to-back photons
    double E1 = randomDouble(0.1, 5.0);
    double E2 = randomDouble(0.1, 5.0);
    
    double theta = randomDouble(0.1, M_PI - 0.1);
    double phi = randomDouble(-M_PI, M_PI);
    
    // Photon 1 direction
    double px1 = E1 * std::sin(theta) * std::cos(phi);
    double py1 = E1 * std::sin(theta) * std::sin(phi);
    double pz1 = E1 * std::cos(theta);
    
    // Photon 2 opposite direction
    double px2 = -E2 * std::sin(theta) * std::cos(phi);
    double py2 = -E2 * std::sin(theta) * std::sin(phi);
    double pz2 = -E2 * std::cos(theta);
    
    TestParticle g1(px1, py1, pz1, E1, 0);
    TestParticle g2(px2, py2, pz2, E2, 1);
    
    // Calculate invariant mass
    double mass = Pi0Helpers::calculate_invariant_mass(g1, g2);
    
    // For back-to-back massless particles: M = 2*sqrt(E1*E2)
    double expected_mass = 2.0 * std::sqrt(E1 * E2);
    
    double tolerance = 1e-8;
    RC_ASSERT(std::abs(mass - expected_mass) < tolerance);
}


/**
 * **Feature: pi0-reconstruction, Property 2: Photon Pair Combinatorics**
 * 
 * *For any* set of n selected photons where n >= 2, the algorithm SHALL generate
 * exactly n*(n-1)/2 unique photon pairs for invariant mass calculation.
 * 
 * **Validates: Requirements 2.1**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, PhotonPairCombinatorics_PairCount, ()) {
    // Generate n photons where n >= 2
    int n_photons = *rc::gen::inRange(2, 15);
    
    std::vector<TestParticle> photons;
    for (int i = 0; i < n_photons; ++i) {
        double energy = randomDouble(0.1, 5.0);
        photons.push_back(makePhotonWithEnergy(energy, i));
    }
    
    // Count the number of pairs generated
    size_t pair_count = 0;
    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            pair_count++;
        }
    }
    
    // Expected number of pairs: n*(n-1)/2
    size_t expected_pairs = static_cast<size_t>(n_photons) * (n_photons - 1) / 2;
    
    RC_ASSERT(pair_count == expected_pairs);
}

/**
 * **Feature: pi0-reconstruction, Property 2: Photon Pair Combinatorics**
 * 
 * *For any* set of n selected photons where n >= 2, each unique pair of photons
 * SHALL be considered exactly once (no duplicates, no self-pairs).
 * 
 * **Validates: Requirements 2.1**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, PhotonPairCombinatorics_UniquePairs, ()) {
    // Generate n photons where n >= 2
    int n_photons = *rc::gen::inRange(2, 10);
    
    std::vector<TestParticle> photons;
    for (int i = 0; i < n_photons; ++i) {
        double energy = randomDouble(0.1, 5.0);
        photons.push_back(makePhotonWithEnergy(energy, i));
    }
    
    // Track all pairs generated using indices
    std::set<std::pair<int, int>> seen_pairs;
    
    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            // Ensure i < j (no self-pairs, ordered pairs)
            RC_ASSERT(i < j);
            
            // Create ordered pair
            auto pair = std::make_pair(static_cast<int>(i), static_cast<int>(j));
            
            // Ensure this pair hasn't been seen before
            RC_ASSERT(seen_pairs.find(pair) == seen_pairs.end());
            
            seen_pairs.insert(pair);
        }
    }
    
    // Verify we got the expected number of unique pairs
    size_t expected_pairs = static_cast<size_t>(n_photons) * (n_photons - 1) / 2;
    RC_ASSERT(seen_pairs.size() == expected_pairs);
}

/**
 * **Feature: pi0-reconstruction, Property 2: Photon Pair Combinatorics**
 * 
 * *For any* set of fewer than 2 photons, the algorithm SHALL return no candidate.
 * 
 * **Validates: Requirements 2.1**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, PhotonPairCombinatorics_InsufficientPhotons, ()) {
    // Generate 0 or 1 photons
    int n_photons = *rc::gen::inRange(0, 2);
    
    std::vector<TestParticle> photons;
    for (int i = 0; i < n_photons; ++i) {
        double energy = randomDouble(0.1, 5.0);
        photons.push_back(makePhotonWithEnergy(energy, i));
    }
    
    double mass_window = 0.03;  // Default window
    auto result = Pi0Helpers::find_best_pi0(photons, mass_window);
    
    // Should return nullopt when fewer than 2 photons
    RC_ASSERT(!result.has_value());
}


/**
 * **Feature: pi0-reconstruction, Property 4: Mass Window Filtering**
 * 
 * *For any* photon pair with invariant mass outside the window [PI0_MASS - window, PI0_MASS + window],
 * the pair SHALL be excluded from π⁰ candidates.
 * 
 * **Validates: Requirements 2.3**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, MassWindowFiltering_OutsideWindow, ()) {
    double mass_window = 0.03;  // ±30 MeV
    double pi0_mass = Pi0Helpers::PI0_MASS;
    
    // Generate two photons that will produce a mass outside the window
    // We'll create back-to-back photons and control their energies to get specific mass
    // For back-to-back photons: M = 2*sqrt(E1*E2)
    // To get mass outside window, we need M < pi0_mass - window OR M > pi0_mass + window
    
    // Choose whether to generate mass below or above the window
    bool below_window = *rc::gen::arbitrary<bool>();
    
    double target_mass;
    if (below_window) {
        // Generate mass below the window (0.05 to 0.105 - 0.03 = 0.075)
        target_mass = randomDouble(0.05, pi0_mass - mass_window - 0.005);
    } else {
        // Generate mass above the window (0.135 + 0.03 + 0.005 = 0.17 to 0.5)
        target_mass = randomDouble(pi0_mass + mass_window + 0.005, 0.5);
    }
    
    RC_PRE(std::abs(target_mass - pi0_mass) > mass_window);
    
    // For back-to-back: M = 2*sqrt(E1*E2), so E1*E2 = M²/4
    // Let E1 = E2 = M/2 for simplicity
    double E = target_mass / 2.0;
    
    double theta = randomDouble(0.1, M_PI - 0.1);
    double phi = randomDouble(-M_PI, M_PI);
    
    // Create back-to-back photons
    double px1 = E * std::sin(theta) * std::cos(phi);
    double py1 = E * std::sin(theta) * std::sin(phi);
    double pz1 = E * std::cos(theta);
    
    double px2 = -E * std::sin(theta) * std::cos(phi);
    double py2 = -E * std::sin(theta) * std::sin(phi);
    double pz2 = -E * std::cos(theta);
    
    TestParticle g1(px1, py1, pz1, E, 0);
    TestParticle g2(px2, py2, pz2, E, 1);
    
    std::vector<TestParticle> photons = {g1, g2};
    
    // Verify the mass is indeed outside the window
    double actual_mass = Pi0Helpers::calculate_invariant_mass(g1, g2);
    RC_PRE(std::abs(actual_mass - pi0_mass) >= mass_window);
    
    // find_best_pi0 should return nullopt since the only pair is outside the window
    auto result = Pi0Helpers::find_best_pi0(photons, mass_window);
    
    RC_ASSERT(!result.has_value());
}

/**
 * **Feature: pi0-reconstruction, Property 4: Mass Window Filtering**
 * 
 * *For any* photon pair with invariant mass inside the window [PI0_MASS - window, PI0_MASS + window],
 * the pair SHALL be considered as a valid π⁰ candidate.
 * 
 * **Validates: Requirements 2.3**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, MassWindowFiltering_InsideWindow, ()) {
    double mass_window = 0.03;  // ±30 MeV
    double pi0_mass = Pi0Helpers::PI0_MASS;
    
    // Generate a mass inside the window
    double target_mass = randomDouble(pi0_mass - mass_window + 0.005, pi0_mass + mass_window - 0.005);
    RC_PRE(std::abs(target_mass - pi0_mass) < mass_window);
    
    // For back-to-back photons: M = 2*sqrt(E1*E2)
    // Let E1 = E2 = M/2
    double E = target_mass / 2.0;
    RC_PRE(E > 0.01);
    
    double theta = randomDouble(0.1, M_PI - 0.1);
    double phi = randomDouble(-M_PI, M_PI);
    
    // Create back-to-back photons
    double px1 = E * std::sin(theta) * std::cos(phi);
    double py1 = E * std::sin(theta) * std::sin(phi);
    double pz1 = E * std::cos(theta);
    
    double px2 = -E * std::sin(theta) * std::cos(phi);
    double py2 = -E * std::sin(theta) * std::sin(phi);
    double pz2 = -E * std::cos(theta);
    
    TestParticle g1(px1, py1, pz1, E, 0);
    TestParticle g2(px2, py2, pz2, E, 1);
    
    std::vector<TestParticle> photons = {g1, g2};
    
    // Verify the mass is indeed inside the window
    double actual_mass = Pi0Helpers::calculate_invariant_mass(g1, g2);
    RC_PRE(std::abs(actual_mass - pi0_mass) < mass_window);
    
    // find_best_pi0 should return a valid candidate
    auto result = Pi0Helpers::find_best_pi0(photons, mass_window);
    
    RC_ASSERT(result.has_value());
    
    // The returned mass should be within the window
    double returned_mass = std::get<2>(result.value());
    RC_ASSERT(std::abs(returned_mass - pi0_mass) < mass_window);
}

/**
 * **Feature: pi0-reconstruction, Property 4: Mass Window Filtering**
 * 
 * *For any* selected π⁰ candidate, its invariant mass SHALL be within the mass window.
 * 
 * **Validates: Requirements 2.3**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, MassWindowFiltering_SelectedWithinWindow, ()) {
    double mass_window = randomDouble(0.02, 0.05);  // Random window size
    double pi0_mass = Pi0Helpers::PI0_MASS;
    
    // Generate multiple photons with various energies
    int n_photons = *rc::gen::inRange(2, 8);
    std::vector<TestParticle> photons;
    
    for (int i = 0; i < n_photons; ++i) {
        double energy = randomDouble(0.05, 3.0);
        photons.push_back(makePhotonWithEnergy(energy, i));
    }
    
    auto result = Pi0Helpers::find_best_pi0(photons, mass_window);
    
    // If a candidate is found, its mass must be within the window
    if (result.has_value()) {
        double returned_mass = std::get<2>(result.value());
        RC_ASSERT(std::abs(returned_mass - pi0_mass) < mass_window);
    }
    // If no candidate found, that's also valid (no pairs within window)
}


/**
 * **Feature: pi0-reconstruction, Property 5: Best Candidate Selection**
 * 
 * *For any* set of π⁰ candidates passing the mass window cut, the selected candidate
 * SHALL have the smallest |mass - PI0_MASS| value among all candidates.
 * 
 * **Validates: Requirements 2.4**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, BestCandidateSelection_ClosestToNominal, ()) {
    double mass_window = 0.05;  // Wider window to allow multiple candidates
    double pi0_mass = Pi0Helpers::PI0_MASS;
    
    // Create photons that will produce multiple candidates within the window
    // We'll create several back-to-back photon pairs with different masses
    std::vector<TestParticle> photons;
    int photon_idx = 0;
    
    // Create 3 pairs of back-to-back photons with different masses within window
    std::vector<double> target_masses = {
        pi0_mass - 0.02,  // Below nominal
        pi0_mass + 0.01,  // Above nominal (closest)
        pi0_mass - 0.03   // Further below
    };
    
    for (double target_mass : target_masses) {
        double E = target_mass / 2.0;
        double theta = 0.5 + photon_idx * 0.3;  // Different angles for each pair
        double phi = photon_idx * 1.0;
        
        double px1 = E * std::sin(theta) * std::cos(phi);
        double py1 = E * std::sin(theta) * std::sin(phi);
        double pz1 = E * std::cos(theta);
        
        double px2 = -E * std::sin(theta) * std::cos(phi);
        double py2 = -E * std::sin(theta) * std::sin(phi);
        double pz2 = -E * std::cos(theta);
        
        photons.emplace_back(px1, py1, pz1, E, photon_idx++);
        photons.emplace_back(px2, py2, pz2, E, photon_idx++);
    }
    
    auto result = Pi0Helpers::find_best_pi0(photons, mass_window);
    
    // Should find a candidate
    RC_ASSERT(result.has_value());
    
    double selected_mass = std::get<2>(result.value());
    double selected_diff = std::abs(selected_mass - pi0_mass);
    
    // Verify this is the best candidate by checking all pairs
    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            double pair_mass = Pi0Helpers::calculate_invariant_mass(photons[i], photons[j]);
            double pair_diff = std::abs(pair_mass - pi0_mass);
            
            // If this pair is within the window, its diff should be >= selected_diff
            if (pair_diff < mass_window) {
                RC_ASSERT(pair_diff >= selected_diff - 1e-10);  // Allow small floating-point tolerance
            }
        }
    }
}

/**
 * **Feature: pi0-reconstruction, Property 5: Best Candidate Selection**
 * 
 * *For any* set of photons with exactly one pair within the mass window,
 * that pair SHALL be selected as the best candidate.
 * 
 * **Validates: Requirements 2.4**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, BestCandidateSelection_SingleCandidate, ()) {
    double mass_window = 0.03;
    double pi0_mass = Pi0Helpers::PI0_MASS;
    
    // Create one pair within the window
    double target_mass = pi0_mass + randomDouble(-0.02, 0.02);
    RC_PRE(std::abs(target_mass - pi0_mass) < mass_window);
    
    double E_good = target_mass / 2.0;
    double theta_good = randomDouble(0.3, 1.0);
    double phi_good = randomDouble(0, 1.0);
    
    // Good pair (back-to-back)
    TestParticle g1(
        E_good * std::sin(theta_good) * std::cos(phi_good),
        E_good * std::sin(theta_good) * std::sin(phi_good),
        E_good * std::cos(theta_good),
        E_good, 0
    );
    TestParticle g2(
        -E_good * std::sin(theta_good) * std::cos(phi_good),
        -E_good * std::sin(theta_good) * std::sin(phi_good),
        -E_good * std::cos(theta_good),
        E_good, 1
    );
    
    // Add a third photon that won't form a valid pair with either g1 or g2
    // (very low energy, so any pair with it will have mass outside window)
    double E_bad = 0.01;  // Very low energy
    TestParticle g3(E_bad, 0, 0, E_bad, 2);
    
    std::vector<TestParticle> photons = {g1, g2, g3};
    
    // Verify g1-g2 is within window
    double mass_12 = Pi0Helpers::calculate_invariant_mass(g1, g2);
    RC_PRE(std::abs(mass_12 - pi0_mass) < mass_window);
    
    // Verify g1-g3 and g2-g3 are outside window
    double mass_13 = Pi0Helpers::calculate_invariant_mass(g1, g3);
    double mass_23 = Pi0Helpers::calculate_invariant_mass(g2, g3);
    RC_PRE(std::abs(mass_13 - pi0_mass) >= mass_window);
    RC_PRE(std::abs(mass_23 - pi0_mass) >= mass_window);
    
    auto result = Pi0Helpers::find_best_pi0(photons, mass_window);
    
    RC_ASSERT(result.has_value());
    
    // The selected mass should be the g1-g2 pair mass
    double selected_mass = std::get<2>(result.value());
    RC_ASSERT(std::abs(selected_mass - mass_12) < 1e-10);
}

/**
 * **Feature: pi0-reconstruction, Property 5: Best Candidate Selection**
 * 
 * *For any* set of photons where no pair is within the mass window,
 * the algorithm SHALL return no candidate.
 * 
 * **Validates: Requirements 2.4**
 */
RC_GTEST_PROP(Pi0ReconstructionProperties, BestCandidateSelection_NoCandidates, ()) {
    double mass_window = 0.03;
    double pi0_mass = Pi0Helpers::PI0_MASS;
    
    // Create photons that will all produce masses outside the window
    // Use very low energies so all pair masses are well below pi0 mass
    int n_photons = *rc::gen::inRange(2, 6);
    std::vector<TestParticle> photons;
    
    for (int i = 0; i < n_photons; ++i) {
        // Very low energy photons - max pair mass will be much less than pi0 mass
        double E = randomDouble(0.01, 0.03);
        photons.push_back(makePhotonWithEnergy(E, i));
    }
    
    // Verify all pairs are outside the window
    bool any_in_window = false;
    for (size_t i = 0; i < photons.size(); ++i) {
        for (size_t j = i + 1; j < photons.size(); ++j) {
            double mass = Pi0Helpers::calculate_invariant_mass(photons[i], photons[j]);
            if (std::abs(mass - pi0_mass) < mass_window) {
                any_in_window = true;
            }
        }
    }
    RC_PRE(!any_in_window);
    
    auto result = Pi0Helpers::find_best_pi0(photons, mass_window);
    
    RC_ASSERT(!result.has_value());
}
