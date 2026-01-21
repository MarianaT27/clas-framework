# Modern Electron Analysis Example

This example demonstrates the modernized CLAS12 analysis framework using C++20/23 features. It showcases the new APIs that reduce boilerplate and improve code clarity.

## Key Features Demonstrated

- **SimpleAlgorithm CRTP base class** - Reduces boilerplate with automatic service injection
- **Range adaptors** - Composable, lazy-evaluated particle filtering
- **CutBuilder** - Fluent, chainable cut definitions with automatic cutflow tracking
- **TypedEventStore** - Type-safe inter-algorithm communication
- **Property system** - Declarative configuration with validation

## Algorithm Chain

The analysis consists of three algorithms:

1. **ModernElectronSelectionAlg** - Selects trigger electrons using range adaptors and CutBuilder
2. **DISKinematicsAlg** - Computes DIS kinematics from selected electrons
3. **ElectronHistogramAlg** - Fills additional analysis histograms

## Code Walkthrough

### 1. Electron Selection with Range Adaptors and CutBuilder

```cpp
class ModernElectronSelectionAlg : public SimpleAlgorithm<ModernElectronSelectionAlg> {
public:
    ModernElectronSelectionAlg()
        : SimpleAlgorithm("ModernElectronSelection")
        , m_cutflow("Electron Selection") {
        // Declare properties with validation
        declare_property(m_min_momentum);
        declare_property(m_max_momentum);
        declare_property(m_min_theta);
        declare_property(m_max_theta);
    }

    auto configure() -> void override {
        // Automatic service injection
        m_hist_svc = &require_service<HistogramService>("HistogramService");
    }

    auto process(Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        // Use range adaptors for composable filtering
        auto fd_electrons = event.all_particles()
            | Ranges::by_pid(CLAS::PIDs::ELECTRON)
            | Ranges::in_detector(CLAS::FD);

        // Collect into vector
        std::vector<CLAS::Particle> electron_candidates;
        for (const auto& e : fd_electrons) {
            electron_candidates.push_back(e);
        }

        if (electron_candidates.empty()) {
            return StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        // Find trigger electron (highest momentum)
        auto trigger_it = std::ranges::max_element(electron_candidates,
            [](const auto& a, const auto& b) { return a.p() < b.p(); });

        const auto& trigger_electron = *trigger_it;
        const double p = trigger_electron.p();
        const double theta = trigger_electron.theta();
        const double vz = trigger_electron.vz();

        // Apply cuts using fluent CutBuilder API
        bool passed = CutBuilder(m_cutflow)
            .require_in_range("Momentum Cut", p, m_min_momentum.get(), m_max_momentum.get())
            .require_in_range("Theta Cut", theta, m_min_theta.get(), m_max_theta.get())
            .require_in_range("Vertex Z Cut", vz, m_vz_min.get(), m_vz_max.get())
            .evaluate();

        if (!passed) {
            return StatusCode::FILTERED();
        }

        // Store in TypedEventStore for downstream algorithms
        m_event_store.store(Keys::TriggerElectron, trigger_electron);
        m_event_store.store(Keys::HasGoodElectron, true);
        event.store("electron_store", m_event_store);

        return StatusCode::SUCCESS();
    }

private:
    // Properties with validation
    Property<double> m_min_momentum{"min_momentum", 1.0, "Minimum electron momentum [GeV]"};
    Property<double> m_max_momentum{"max_momentum", 10.5, "Maximum electron momentum [GeV]"};
    Property<double> m_min_theta{"min_theta", 5.0, "Minimum theta angle [deg]"};
    Property<double> m_max_theta{"max_theta", 35.0, "Maximum theta angle [deg]"};
    Property<double> m_vz_min{"vz_min", -12.0, "Minimum vertex z [cm]"};
    Property<double> m_vz_max{"vz_max", 10.0, "Maximum vertex z [cm]"};

    ServiceHandle<HistogramService>* m_hist_svc = nullptr;
    Cutflow m_cutflow;
    TypedEventStore m_event_store;
};
```

### 2. DIS Kinematics with TypedEventStore

```cpp
class DISKinematicsAlg : public SimpleAlgorithm<DISKinematicsAlg> {
public:
    DISKinematicsAlg() : SimpleAlgorithm("DISKinematics") {
        declare_property(m_beam_energy);
    }

    auto process(Event& event) -> StatusCode override {
        // Retrieve electron from TypedEventStore
        auto store_opt = event.retrieve<TypedEventStore>("electron_store");
        if (!store_opt) return StatusCode::FILTERED();

        auto& store = *store_opt;
        auto electron_opt = store.retrieve(Keys::TriggerElectron);
        if (!electron_opt) return StatusCode::FILTERED();

        const auto& electron = *electron_opt;

        // Compute DIS kinematics
        auto dis_kin = Physics::KinematicCalculator::calculate_DIS(beam, electron, target);

        // Store computed kinematics for downstream use
        TypedEventStore kinematics_store;
        kinematics_store.store(Keys::Q2, dis_kin.Q2);
        kinematics_store.store(Keys::W, dis_kin.W);
        kinematics_store.store(Keys::Xbj, dis_kin.x_bj);
        event.store("kinematics_store", kinematics_store);

        return StatusCode::SUCCESS();
    }

private:
    Property<double> m_beam_energy{"beam_energy", 10.6, "Beam energy [GeV]"};
};
```

## Running the Example

```bash
# Build the example
meson setup builddir
ninja -C builddir example_modern_electron_analysis

# Run with default data directory
./builddir/example_modern_electron_analysis

# Run with custom input
./builddir/example_modern_electron_analysis -i /path/to/data.hipo -o output.root

# Show help
./builddir/example_modern_electron_analysis --help
```

## Output

The analysis produces:

1. **Cutflow table** showing selection efficiency at each stage
2. **ROOT file** with histograms organized by category:
   - `/Selection/` - Before/after cut distributions
   - `/Kinematics/` - Q², W, x_Bj, y distributions
   - `/Analysis/` - Additional analysis histograms

Example cutflow output:

```
════════════════════════════════════════════════════════════
  Cutflow: Electron Selection
════════════════════════════════════════════════════════════
Cut                                     Events  Abs Eff  Rel Eff
────────────────────────────────────────────────────────────
All Events                                 198   100.0%   100.0%
Has Electron                               106    53.5%    53.5%
Forward Detector                           106    53.5%   100.0%
Momentum Cut                               106    53.5%   100.0%
Theta Cut                                  106    53.5%   100.0%
Vertex Z Cut                                56    28.3%    52.8%
════════════════════════════════════════════════════════════
  Overall efficiency: 28.28%
```

## Comparison: Old vs Modern API

### Old API (Traditional)

```cpp
class ElectronAnalysis : public Framework::Algorithm {
    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) return StatusCode::SUCCESS();
        
        // Manual filtering
        const CLAS::Particle* trigger = nullptr;
        for (const auto& e : electrons) {
            if (std::abs(e.status()) >= 2000 && std::abs(e.status()) < 4000) {
                trigger = &e;
                break;
            }
        }
        
        // Manual cut application
        if (!CLAS::ElectronCuts::pass_electron_momentum_cut(trigger->p()))
            return StatusCode::SUCCESS();
        m_cutflow.pass("Momentum");
        
        // ... more manual cuts
    }
};
```

### Modern API (C++20/23)

```cpp
class ModernElectronSelectionAlg : public SimpleAlgorithm<ModernElectronSelectionAlg> {
    auto process(Event& event) -> StatusCode override {
        // Composable range adaptors
        auto fd_electrons = event.all_particles()
            | Ranges::by_pid(CLAS::PIDs::ELECTRON)
            | Ranges::in_detector(CLAS::FD);
        
        // Fluent cut builder
        bool passed = CutBuilder(m_cutflow)
            .require_in_range("Momentum Cut", p, m_min_momentum.get(), m_max_momentum.get())
            .require_in_range("Theta Cut", theta, m_min_theta.get(), m_max_theta.get())
            .evaluate();
        
        // Type-safe event store
        m_event_store.store(Keys::TriggerElectron, trigger_electron);
    }
};
```

## Benefits of the Modern API

| Feature | Benefit |
|---------|---------|
| SimpleAlgorithm | Automatic service injection, reduced boilerplate |
| Range Adaptors | Composable, lazy-evaluated, expressive filtering |
| CutBuilder | Fluent API, automatic cutflow tracking |
| TypedEventStore | Compile-time type safety, clear data flow |
| Property System | Declarative configuration, automatic validation |
