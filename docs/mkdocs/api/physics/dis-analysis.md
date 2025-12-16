# DIS Analysis

Deep Inelastic Scattering analysis utilities.

## DISAnalysis

```cpp
namespace Physics {

struct DISKinematics {
    double q2;    // Q² [GeV²]
    double w;     // W [GeV]
    double nu;    // ν [GeV]
    double xb;    // Bjorken x
    double y;     // Inelasticity
};

class DISAnalysis {
public:
    static auto calculate_kinematics(const Particle& electron) -> DISKinematics;
    static auto calculate_kinematics(const Particle& beam, 
                                     const Particle& electron,
                                     const Particle& target) -> DISKinematics;
};

}
```

## Usage

```cpp
// Simple interface (uses default beam/target)
auto kin = Physics::DISAnalysis::calculate_kinematics(electron);

std::cout << "Q² = " << kin.q2 << " GeV²\n";
std::cout << "W = " << kin.w << " GeV\n";
std::cout << "x_B = " << kin.xb << "\n";
std::cout << "y = " << kin.y << "\n";

// Full interface
CLAS::Particle beam(11, 0, -1, -1, CLAS::Constants::ELECTRONMASS,
                    0, 0, 10.53, 10.53, 0, 0, 0, 0, 1.0, 0);
CLAS::Particle target(2212, 0, -1, 1, CLAS::Constants::PROTONMASS,
                      0, 0, 0, CLAS::Constants::PROTONMASS, 0, 0, 0, 0, 0, 0);

auto kin = Physics::DISAnalysis::calculate_kinematics(beam, electron, target);
```

## DIS Cuts

Typical DIS selection:

```cpp
auto kin = Physics::DISAnalysis::calculate_kinematics(electron);

// Standard DIS cuts
bool pass_q2 = kin.q2 > 1.0;           // Q² > 1 GeV²
bool pass_w = kin.w > 2.0;             // W > 2 GeV (DIS region)
bool pass_y = kin.y < 0.85;            // y < 0.85 (radiative corrections)

if (pass_q2 && pass_w && pass_y) {
    // Event is in DIS region
}
```
