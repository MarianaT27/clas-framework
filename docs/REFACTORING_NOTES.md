# Core Module Refactoring Notes

## Overview

The `Core` module has been refactored to improve organization and separation of concerns. Functions and classes have been moved to more appropriate locations based on their purpose.

## New Module Structure

### Core Utilities

1. **`include/Core/FileUtils.hpp`** - File and directory utilities
   - `CLAS::FileUtils::read_recursive_hipo_files()` - Read HIPO files recursively
   - `CLAS::FileUtils::read_subdirectories()` - List subdirectories
   - `CLAS::FileUtils::filter_by_run_numbers()` - Filter files by run numbers

2. **`include/Core/StringUtils.hpp`** - String manipulation utilities
   - `CLAS::StringUtils::format_number()` - Format numbers with precision
   - `CLAS::StringUtils::replace_all()` - Replace all occurrences of a substring

3. **`include/Core/ROOTUtils.hpp`** - ROOT-specific utilities
   - `CLAS::ROOTUtils::configure_ROOT()` - Configure ROOT global options
   - `CLAS::ROOTUtils::find_equal_count_bins()` - Find optimal bin edges

### Physics Utilities

4. **`include/Physics/ParticleUtils.hpp`** - Physics-related particle utilities
   - `Physics::ParticleUtils::get_mass()` - Get particle mass from PDG ID
   - `Physics::ParticleUtils::compute_energy()` - Compute relativistic energy
   - `Physics::ParticleUtils::find_trigger_electron()` - Find trigger electron
   - `Physics::ParticleUtils::find_most_energetic_electron()` - Find highest energy electron
   - `Physics::ParticleUtils::generate_unique_pairs()` - Generate unique particle pairs
   - `Physics::ParticleUtils::mod()` - Floating-point modulo
   - `Physics::ParticleUtils::wrap_angle_deg()` - Wrap angle to [-180, 180]
   - `Physics::ParticleUtils::wrap_angle_rad()` - Wrap angle to [-π, π]

5. **`include/Physics/Kinematics.hpp`** - Kinematic calculations (existing)
   - `Physics::KinematicCalculator` - Q², W, x_Bjorken, missing mass, etc.

### Convenience Header

- **`include/Core/Helpers.hpp`** - Includes all utility modules for convenience

## Files Removed

- **`include/Core/Kinematics.hpp`** - Removed (use `Physics/Kinematics.hpp` instead)

## Files Updated

- All source files updated to use new APIs
- `include/Core/ParticleSelector.hpp` - Uses `Physics::ParticleUtils`
- `include/Core/ElectronCuts.hpp` - Electron selection cuts
- `include/Physics/DISAnalysis.hpp` - Uses `Physics::KinematicCalculator`

## Usage Examples

### File Operations
```cpp
#include <CLAS/FileUtils.hpp>

auto files = CLAS::FileUtils::read_recursive_hipo_files("/data/hipo");
auto dirs = CLAS::FileUtils::read_subdirectories("/data");
auto filtered = CLAS::FileUtils::filter_by_run_numbers(files, {"5032", "5033"});
```

### Physics Calculations
```cpp
#include <Physics/ParticleUtils.hpp>

double mass = Physics::ParticleUtils::get_mass(11);  // electron mass
double E = Physics::ParticleUtils::compute_energy(px, py, pz, 11);
auto trigger = Physics::ParticleUtils::find_trigger_electron(electrons, {2000, 4000});
auto pairs = Physics::ParticleUtils::generate_unique_pairs(photons);
```

### Kinematics
```cpp
#include <Physics/Kinematics.hpp>

double Q2 = Physics::KinematicCalculator::calculate_Q2(beam, electron);
double W = Physics::KinematicCalculator::calculate_W(beam, electron, target);
```

### ROOT Configuration
```cpp
#include <CLAS/ROOTUtils.hpp>

CLAS::ROOTUtils::configure_ROOT();
CLAS::ROOTUtils::find_equal_count_bins(values, 6);
```

## ParticleSelectorService

A new service `Framework::ParticleSelectorService` has been added for particle selection:

```cpp
#include <Framework/Services/ParticleSelectorService.hpp>

// Setup
auto particle_svc = std::make_unique<Framework::ParticleSelectorService>();
app.service_manager().add_service(std::move(particle_svc));

// Usage in algorithm
m_particle_svc = service<Framework::ParticleSelectorService>("ParticleSelectorService");
auto particles = m_particle_svc->extract_particles(bank);
auto electrons = m_particle_svc->get_electrons(particles);
auto filtered = m_particle_svc->with_momentum_range(electrons, 1.0, 10.0);
```

Particle selection is now handled exclusively through `Framework::ParticleSelectorService`, which provides a service-based, configurable approach that can be shared across algorithms.

## Rationale

1. **Separation of Concerns**: Physics-related functions belong in the `Physics` namespace
2. **Modularity**: Smaller, focused headers compile faster and are easier to maintain
3. **Discoverability**: Functions are in namespaces that match their purpose
4. **Service Architecture**: Services integrate with the framework for shared, configurable functionality
5. **Bug Fixes**: Fixed NaN comparison bug in `Cuts.hpp` (`== quiet_NaN()` → `std::isnan()`)

## Cut Services

The cuts have been refactored into services for better integration with the framework:

### ElectronCutsService
```cpp
#include <Framework/Services/ElectronCutsService.hpp>

auto electron_cuts = service<Framework::ElectronCutsService>("ElectronCutsService");
electron_cuts->set_target(CLAS::Target::LD2);
electron_cuts->set_polarity(CLAS::Polarity::Outbending);

if (electron_cuts->pass_chi2pid_cut(electron.chi2pid()) &&
    electron_cuts->pass_momentum_cut(electron.p()) &&
    electron_cuts->pass_vz_cut(electron.vz())) {
    // Electron passes basic cuts
}
```

### PhotonCutsService
```cpp
#include <Framework/Services/PhotonCutsService.hpp>

auto photon_cuts = service<Framework::PhotonCutsService>("PhotonCutsService");
photon_cuts->set_pass(1);  // Reconstruction pass

auto good_photons = photon_cuts->select_photons_with_AI(
    RUN_config, REC_Particle, REC_Calorimeter);
```

### SamplingFractionService
```cpp
#include <Framework/Services/SamplingFractionService.hpp>

auto sf_svc = service<Framework::SamplingFractionService>("SamplingFractionService");

bool passes = sf_svc->passes_cut(SF, momentum, sector, 
    CLAS::Target::LD2, CLAS::Polarity::Outbending);
```

The `CLAS::ElectronCuts` class provides static utility methods for individual cuts, while `SamplingFractionService` handles the target/polarity-dependent sampling fraction parameterization. All algorithms should use the service-based approach for sampling fraction cuts.
