# Kinematics

Kinematic calculations for particle physics.

## KinematicCalculator

```cpp
namespace Physics {

class KinematicCalculator {
public:
    // DIS variables
    static auto calculate_Q2(const Particle& beam, const Particle& electron) -> double;
    static auto calculate_W(const Particle& beam, const Particle& electron, 
                           const Particle& target) -> double;
    static auto calculate_y(const Particle& beam, const Particle& electron) -> double;
    static auto calculate_x_bjorken(double q2, double nu) -> double;
    
    // Exclusive reactions
    static auto missing_mass(const Particle& beam, const Particle& target,
                            const std::vector<Particle>& final_state) -> double;
    static auto missing_momentum(const Particle& beam, const Particle& target,
                                const std::vector<Particle>& final_state) -> double;
    
    // Geometry
    static auto opening_angle(const Particle& p1, const Particle& p2) -> double;
    static auto delta_phi(const Particle& p1, const Particle& p2) -> double;
};

}
```

## CompositeParticle

```cpp
namespace Physics {

class CompositeParticle {
public:
    explicit CompositeParticle(std::vector<Particle> daughters);
    
    auto invariant_mass() const -> double;
    auto momentum() const -> ROOT::Math::PxPyPzEVector;
    auto daughters() const -> const std::vector<Particle>&;
};

}
```

## Usage

```cpp
// DIS kinematics
double q2 = Physics::KinematicCalculator::calculate_Q2(beam, electron);
double w = Physics::KinematicCalculator::calculate_W(beam, electron, target);

// π⁰ reconstruction
Physics::CompositeParticle pi0({photon1, photon2});
double m_gg = pi0.invariant_mass();

// Missing mass
double mm = Physics::KinematicCalculator::missing_mass(
    beam, target, {electron, proton}
);
```
