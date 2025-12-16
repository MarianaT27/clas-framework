# Particle

CLAS12 particle class with kinematics and identification.

## Class Definition

```cpp
namespace CLAS {

class Particle {
public:
    Particle() = default;
    Particle(int pdg, int status, int index, int charge, double mass,
             double px, double py, double pz, double E,
             double vx, double vy, double vz, double vt,
             double beta, double chi2pid);
    
    // Identification
    auto pdg() const -> int;
    auto status() const -> int;
    auto index() const -> int;
    auto charge() const -> int;
    
    // Kinematics
    auto px() const -> double;
    auto py() const -> double;
    auto pz() const -> double;
    auto E() const -> double;
    auto mass() const -> double;
    auto p() const -> double;      // Momentum magnitude
    auto pt() const -> double;     // Transverse momentum
    auto theta() const -> double;  // Polar angle [deg]
    auto phi() const -> double;    // Azimuthal angle [deg]
    
    // Vertex
    auto vx() const -> double;
    auto vy() const -> double;
    auto vz() const -> double;
    auto vt() const -> double;
    
    // PID
    auto beta() const -> double;
    auto chi2pid() const -> double;
    
    // ROOT vectors
    auto PxPyPzEVector() const -> ROOT::Math::PxPyPzEVector;
    auto PxPyPzVector() const -> ROOT::Math::XYZVector;
    auto VxVyVzVector() const -> ROOT::Math::XYZVector;
    
    // Validation
    auto is_valid() const -> bool;
};

}
```

## Usage

```cpp
const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);

for (const auto& e : electrons) {
    // Kinematics
    double momentum = e.p();
    double theta = e.theta();
    double phi = e.phi();
    
    // Vertex
    double vz = e.vz();
    
    // PID quality
    double chi2 = e.chi2pid();
    
    // ROOT 4-vector
    auto p4 = e.PxPyPzEVector();
}
```

## Particle IDs

```cpp
namespace CLAS {
enum class PIDs : int {
    ELECTRON = 11,
    POSITRON = -11,
    PHOTON = 22,
    PROTON = 2212,
    NEUTRON = 2112,
    PIPLUS = 211,
    PIMINUS = -211,
    KPLUS = 321,
    KMINUS = -321
};
}
```
