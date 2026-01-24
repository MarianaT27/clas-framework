#include <gtest/gtest.h>
#include <vector>
#include <ranges>
#include <algorithm>

#include <clas12/framework/core/Ranges.hpp>
#include <clas12/Particle.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Detectors.hpp>

using namespace clas12::framework::Ranges;
using namespace clas12;

// Helper function to create test particles
auto make_particle(int pdg, int status, int index, int charge, 
                   double px, double py, double pz, double E,
                   double vx = 0.0, double vy = 0.0, double vz = 0.0) -> Particle {
    double mass = 0.0;
    if (pdg == 11 || pdg == -11) mass = 0.000511;  // electron/positron
    else if (pdg == 2212) mass = 0.938272;  // proton
    else if (pdg == 22) mass = 0.0;  // photon
    else if (pdg == 211 || pdg == -211) mass = 0.13957;  // pion
    
    return Particle(pdg, status, index, charge, mass, px, py, pz, E, vx, vy, vz, 0.0, 1.0, 0.0);
}

class RangesTest : public ::testing::Test {
protected:
    std::vector<Particle> particles;
    
    void SetUp() override {
        // Create a diverse set of test particles
        // Electrons in FD (status 2000-3999)
        particles.push_back(make_particle(11, 2000, 0, -1, 1.0, 0.5, 3.0, 3.2));  // p ~ 3.2 GeV
        particles.push_back(make_particle(11, 2500, 1, -1, 0.5, 0.3, 1.5, 1.6));  // p ~ 1.6 GeV
        
        // Electron in CD (status 4000-4999)
        particles.push_back(make_particle(11, 4000, 2, -1, 0.3, 0.2, 0.8, 0.9));  // p ~ 0.9 GeV
        
        // Protons in FD
        particles.push_back(make_particle(2212, 2100, 3, 1, 0.5, 0.3, 1.0, 1.4));  // p ~ 1.2 GeV
        particles.push_back(make_particle(2212, 2200, 4, 1, 0.8, 0.4, 2.0, 2.3));  // p ~ 2.2 GeV
        
        // Photons in FD
        particles.push_back(make_particle(22, 2000, 5, 0, 0.5, 0.5, 1.0, 1.2));   // p ~ 1.2 GeV
        particles.push_back(make_particle(22, 2100, 6, 0, 1.0, 1.0, 2.0, 2.4));   // p ~ 2.4 GeV
        
        // Pions
        particles.push_back(make_particle(211, 2000, 7, 1, 0.3, 0.2, 0.5, 0.6));  // pi+
        particles.push_back(make_particle(-211, 2000, 8, -1, 0.4, 0.3, 0.6, 0.8)); // pi-
        
        // Electron in FT (status 1000-1999)
        particles.push_back(make_particle(11, 1500, 9, -1, 0.2, 0.1, 0.5, 0.55)); // p ~ 0.55 GeV
    }
};

// Test by_pid filter
TEST_F(RangesTest, FilterByPid) {
    auto electrons = particles | by_pid(PIDs::ELECTRON);
    int count = 0;
    for (const auto& p : electrons) {
        EXPECT_EQ(p.pdg(), 11);
        count++;
    }
    EXPECT_EQ(count, 4);  // 4 electrons total
}

// Test by_pdg filter with integer
TEST_F(RangesTest, FilterByPdgInt) {
    auto protons = particles | by_pdg(2212);
    int count = 0;
    for (const auto& p : protons) {
        EXPECT_EQ(p.pdg(), 2212);
        count++;
    }
    EXPECT_EQ(count, 2);
}

// Test by_charge filter
TEST_F(RangesTest, FilterByCharge) {
    auto negative = particles | by_charge(-1);
    int count = 0;
    for (const auto& p : negative) {
        EXPECT_EQ(p.charge(), -1);
        count++;
    }
    EXPECT_EQ(count, 5);  // 4 electrons + 1 pi-
    
    auto neutral = particles | by_charge(0);
    int neutral_count = 0;
    for (const auto& p : neutral) {
        EXPECT_EQ(p.charge(), 0);
        neutral_count++;
    }
    EXPECT_EQ(neutral_count, 2);  // 2 photons
}

// Test by_momentum filter
TEST_F(RangesTest, FilterByMomentum) {
    auto high_p = particles | by_momentum(2.0, 10.0);
    int count = 0;
    for (const auto& p : high_p) {
        EXPECT_GE(p.p(), 2.0);
        EXPECT_LE(p.p(), 10.0);
        count++;
    }
    EXPECT_GE(count, 2);  // At least the high-momentum electron and proton
}

// Test by_min_momentum filter
TEST_F(RangesTest, FilterByMinMomentum) {
    auto high_p = particles | by_min_momentum(2.0);
    for (const auto& p : high_p) {
        EXPECT_GE(p.p(), 2.0);
    }
}

// Test in_detector filter for FD
TEST_F(RangesTest, FilterByDetectorFD) {
    auto fd_particles = particles | in_detector(FD);
    int count = 0;
    for (const auto& p : fd_particles) {
        int abs_status = std::abs(p.status());
        EXPECT_GE(abs_status, 2000);
        EXPECT_LT(abs_status, 4000);
        count++;
    }
    EXPECT_EQ(count, 8);  // All except CD electron and FT electron
}

// Test in_detector filter for CD
TEST_F(RangesTest, FilterByDetectorCD) {
    auto cd_particles = particles | in_detector(CD);
    int count = 0;
    for (const auto& p : cd_particles) {
        int abs_status = std::abs(p.status());
        EXPECT_GE(abs_status, 4000);
        EXPECT_LT(abs_status, 5000);
        count++;
    }
    EXPECT_EQ(count, 1);  // Only the CD electron
}

// Test in_detector filter for FT
TEST_F(RangesTest, FilterByDetectorFT) {
    auto ft_particles = particles | in_detector(FT);
    int count = 0;
    for (const auto& p : ft_particles) {
        int abs_status = std::abs(p.status());
        EXPECT_GE(abs_status, 1000);
        EXPECT_LT(abs_status, 2000);
        count++;
    }
    EXPECT_EQ(count, 1);  // Only the FT electron
}

// Test filter composition with pipe operator (Requirement 3.4)
TEST_F(RangesTest, FilterComposition) {
    // Get electrons in FD with momentum > 1.5 GeV
    auto fd_high_p_electrons = particles 
        | by_pid(PIDs::ELECTRON)
        | in_detector(FD)
        | by_min_momentum(1.5);
    
    int count = 0;
    for (const auto& p : fd_high_p_electrons) {
        EXPECT_EQ(p.pdg(), 11);
        EXPECT_GE(p.p(), 1.5);
        int abs_status = std::abs(p.status());
        EXPECT_GE(abs_status, 2000);
        EXPECT_LT(abs_status, 4000);
        count++;
    }
    EXPECT_EQ(count, 2);  // Two FD electrons with p > 1.5 GeV
}

// Test to_lorentz_vector transform (Requirement 3.4)
TEST_F(RangesTest, ToLorentzVector) {
    auto momenta = particles 
        | by_pid(PIDs::ELECTRON)
        | to_lorentz_vector;
    
    int count = 0;
    for (const auto& lv : momenta) {
        // Verify it's a valid 4-vector
        EXPECT_GT(lv.E(), 0);
        count++;
    }
    EXPECT_EQ(count, 4);  // 4 electrons
}

// Test to_momentum transform
TEST_F(RangesTest, ToMomentum) {
    auto momenta = particles 
        | by_pid(PIDs::PROTON)
        | to_momentum;
    
    std::vector<double> p_values;
    for (double p : momenta) {
        p_values.push_back(p);
    }
    EXPECT_EQ(p_values.size(), 2);
    for (double p : p_values) {
        EXPECT_GT(p, 0);
    }
}

// Test complex filter chain
TEST_F(RangesTest, ComplexFilterChain) {
    // Get charged particles in FD with momentum between 1.0 and 3.0 GeV
    auto result = particles
        | std::views::filter([](const Particle& p) { return p.charge() != 0; })
        | in_detector(FD)
        | by_momentum(1.0, 3.0);
    
    int count = 0;
    for (const auto& p : result) {
        EXPECT_NE(p.charge(), 0);
        EXPECT_GE(p.p(), 1.0);
        EXPECT_LE(p.p(), 3.0);
        count++;
    }
    EXPECT_GE(count, 1);
}

// Test that filters are lazy (don't evaluate until iterated)
TEST_F(RangesTest, LazyEvaluation) {
    int filter_calls = 0;
    
    // Create a custom filter that counts calls
    auto counting_filter = std::views::filter([&filter_calls](const Particle& p) {
        filter_calls++;
        return p.pdg() == 11;
    });
    
    // Just creating the view shouldn't call the filter
    auto view = particles | counting_filter;
    EXPECT_EQ(filter_calls, 0);
    
    // Iterating should call the filter
    for (const auto& p : view) {
        (void)p;  // Suppress unused warning
    }
    EXPECT_EQ(filter_calls, static_cast<int>(particles.size()));
}

// Test valid_only filter
TEST_F(RangesTest, ValidOnly) {
    // All our test particles should be valid
    auto valid = particles | valid_only;
    int count = 0;
    for (const auto& p : valid) {
        EXPECT_TRUE(p.is_valid());
        count++;
    }
    EXPECT_EQ(count, static_cast<int>(particles.size()));
}

// Test combining multiple transforms
TEST_F(RangesTest, MultipleTransforms) {
    auto thetas = particles
        | by_pid(PIDs::ELECTRON)
        | to_theta;
    
    std::vector<double> theta_values;
    for (double theta : thetas) {
        theta_values.push_back(theta);
    }
    EXPECT_EQ(theta_values.size(), 4);
}

// Test empty result
TEST_F(RangesTest, EmptyResult) {
    // Filter for a PID that doesn't exist
    auto kaons = particles | by_pid(PIDs::KPLUS);
    int count = 0;
    for (const auto& p : kaons) {
        (void)p;
        count++;
    }
    EXPECT_EQ(count, 0);
}
