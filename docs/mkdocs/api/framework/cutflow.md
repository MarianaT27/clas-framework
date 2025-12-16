# Cutflow

Automatic cut efficiency tracking.

## Class Definition

```cpp
namespace Framework {

class Cutflow {
public:
    explicit Cutflow(std::string name);
    
    auto add_cut(const std::string& cut_name) -> void;
    auto pass(const std::string& cut_name) -> void;
    auto print() const -> void;
    
    auto get_count(const std::string& cut_name) const -> long;
    auto get_efficiency(const std::string& cut_name) const -> double;
};

}
```

## Usage

```cpp
class MyAlgorithm : public Framework::Algorithm {
private:
    Framework::Cutflow m_cutflow{"Electron Selection"};

public:
    auto initialize() -> Framework::StatusCode override {
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Momentum > 1 GeV");
        m_cutflow.add_cut("Chi2 PID");
        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");
        
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");
        
        if (electrons[0].p() < 1.0) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Momentum > 1 GeV");
        
        if (std::abs(electrons[0].chi2pid()) > 3.0) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Chi2 PID");
        
        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }
};
```

## Output

```
═══════════════════════════════════════════════════════════════
  Cutflow: Electron Selection
═══════════════════════════════════════════════════════════════
Cut                                    Events   Abs Eff  Rel Eff
───────────────────────────────────────────────────────────────
All Events                             100000    100.0%    100.0%
Has Electron                            45230     45.2%     45.2%
Momentum > 1 GeV                        42108     42.1%     93.1%
Chi2 PID                                38542     38.5%     91.5%
═══════════════════════════════════════════════════════════════
```
