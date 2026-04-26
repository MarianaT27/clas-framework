# Algorithm Template

This template provides a starting point for creating new physics algorithms using the `Algorithm` base class.

## Features

- **Automatic service injection** via `require_service<T>()`
- **Configurable properties** via `Property<T>`
- **Clear lifecycle hooks**: `configure()`, `on_initialize()`, `process()`, `on_finalize()`

## Usage

### Using the scaffolding script

```bash
# Generate a new algorithm as part of an analysis
python templates/create_analysis.py --name my_analysis --with-algorithm

# The algorithm will be generated as MyAnalysisAlg.hpp
```

### Manual usage

1. Copy `MyAlgorithm.hpp.template` to your analysis directory
2. Replace template variables:
   - `${CLASS_NAME}` - Your algorithm class name (PascalCase)
   - `${NAMESPACE}` - Your C++ namespace
   - `${DESCRIPTION}` - Brief description

## Template Variables

| Variable | Description | Example |
|----------|-------------|---------|
| `${CLASS_NAME}` | Algorithm class name | `ElectronSelectionAlg` |
| `${NAMESPACE}` | C++ namespace | `myanalysis` |
| `${DESCRIPTION}` | Brief description | `Electron selection algorithm` |

## Example

```cpp
#include <clas12/framework/core/Algorithm.hpp>

namespace myanalysis {

class ElectronSelectionAlg : public clas12::framework::Algorithm<ElectronSelectionAlg> {
public:
    ElectronSelectionAlg() : Algorithm("ElectronSelectionAlg") {
        declare_property(m_momentum_cut);
    }

    auto configure() -> void override {
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        auto electrons = event.particles(clas12::PIDs::ELECTRON);
        for (const auto& e : electrons) {
            if (e.p() > m_momentum_cut) {
                m_hist_svc->get()->fill("electron_p", e.p());
            }
        }
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    clas12::framework::Property<double> m_momentum_cut{"momentum_cut", 2.0, "Min momentum [GeV]"};
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
};

}  // namespace myanalysis
```
