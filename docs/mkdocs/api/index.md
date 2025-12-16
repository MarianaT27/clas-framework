# API Reference

This section provides detailed API documentation for the CLAS12 Analysis Framework.

## Namespaces

| Namespace | Description |
|-----------|-------------|
| `Framework` | Core framework classes (Application, Services, Algorithms) |
| `CLAS` | CLAS12-specific utilities (Particle, Cuts, Banks) |
| `Physics` | Physics calculations (Kinematics, DIS Analysis) |
| `Algorithms` | Pre-built analysis algorithms |

## Core Classes

### Framework

- [Application](framework/application.md) - Main application class
- [Algorithm](framework/algorithm.md) - Base algorithm interface
- [Event](framework/event.md) - Event data model
- [Services](framework/services.md) - Service classes
- [Cutflow](framework/cutflow.md) - Cut efficiency tracking

### CLAS

- [Particle](clas/particle.md) - Particle class
- [Cuts](clas/cuts.md) - Physics cuts
- [Banks](clas/banks.md) - HIPO bank access

### Physics

- [Kinematics](physics/kinematics.md) - Kinematic calculations
- [DIS Analysis](physics/dis-analysis.md) - DIS-specific utilities

## Doxygen Documentation

For complete API documentation including all classes, methods, and parameters, see the [Doxygen-generated documentation](../api/index.html).

## Quick Reference

### Creating an Application

```cpp
Framework::Application app("MyAnalysis");
app.set_input_files({"data.hipo"});
app.set_num_threads(4);
app.initialize();
app.run();
app.finalize();
```

### Accessing Particles

```cpp
const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
const auto& photons = event.particles(CLAS::PIDs::PHOTON);
```

### Booking Histograms

```cpp
auto* h = m_hist_svc->book_hist1D("/path/name", "Title;X;Y", 100, 0, 10);
```

### Logging

```cpp
Framework::Log::info(name(), "Message with {} args", value);
```
