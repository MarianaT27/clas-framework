# Services

Framework services for shared functionality.

## Core Services

### HistogramService

All histograms are thread-safe atomic histograms by default.

```cpp
class HistogramService : public IService {
public:
    auto set_output_file(const std::string& filename) -> void;

    // Core booking methods - return atomic histograms
    auto book_hist1D(const std::string& path, const std::string& title,
                     int nbins, double xmin, double xmax) -> ThreadLocalHistogram1D*;
    auto book_hist2D(const std::string& path, const std::string& title,
                     int nbinsx, double xmin, double xmax,
                     int nbinsy, double ymin, double ymax) -> ThreadLocalHistogram2D*;

    // Physics histogram templates
    auto book_momentum_hist(...) -> ThreadLocalHistogram1D*;
    auto book_theta_hist(...) -> ThreadLocalHistogram1D*;
    auto book_phi_hist(...) -> ThreadLocalHistogram1D*;
    auto book_q2_hist(...) -> ThreadLocalHistogram1D*;
    auto book_w_hist(...) -> ThreadLocalHistogram1D*;
    auto book_xb_hist(...) -> ThreadLocalHistogram1D*;
    auto book_vz_hist(...) -> ThreadLocalHistogram1D*;
    auto book_theta_vs_p(...) -> ThreadLocalHistogram2D*;

    // Access methods
    auto get_atomic_hist1D(const std::string& path) -> ThreadLocalHistogram1D*;
    auto get_atomic_hist2D(const std::string& path) -> ThreadLocalHistogram2D*;
    auto get_hist1D(const std::string& path) -> TH1D*;
    auto get_hist2D(const std::string& path) -> TH2D*;
};
```

### LoggingService

```cpp
class LoggingService : public IService {
public:
    auto set_log_level(LogLevel level) -> void;
    auto set_log_file(const std::string& filename) -> void;
    auto debug/info/warning/error(const std::string& source, const std::string& msg) -> void;
};

class Log {  // Global singleton
    static auto info/debug/warning/error(const std::string& source, const std::string& fmt, Args&&...) -> void;
};
```

### NTupleService

```cpp
class NTupleService : public IService {
public:
    auto set_output_file(const std::string& filename) -> void;
    template<typename T>
    auto register_branch(const std::string& tree, const std::string& branch, T* ptr) -> void;
    auto fill(const std::string& tree) -> void;
};
```

### ConditionsService

```cpp
class ConditionsService : public IService {
public:
    auto set_run_number(int run) -> void;
    auto get_time_offset(const std::string& detector, int sector) -> double;
    template<typename T>
    auto set_value(const std::string& key, T value, IntervalOfValidity iov) -> void;
};
```

---

## Physics Analysis Services

### FiducialService

Unified detector fiducial cuts.

```cpp
enum class DetectorRegion { PCAL, ECin, ECout, DC_R1, DC_R2, DC_R3, HTCC, FTOF };

class FiducialService : public Service {
public:
    auto register_pcal_fiducial(double lu_min = 9.0, double lw_min = 9.0) -> void;
    auto register_dc_edge_cuts(double r1_min, double r2_min, double r3_min) -> void;
    auto passes_fiducial(const Particle& p, DetectorRegion region, ...) const -> bool;
    auto passes_all_fiducial(const Particle& p, const BankData& banks) const -> bool;
    auto acceptance_weight(const Particle& p, DetectorRegion region) const -> double;
};
```

## Advanced Fitting Services

### DecayTreeFitterService

LHCb-style global topological decay fitting.

```cpp
struct DTFResult {
    bool converged;
    double chi2, probability;
    int ndf;
    std::map<int, ROOT::Math::PxPyPzEVector> fitted_momenta;
    std::map<int, ROOT::Math::XYZVector> fitted_vertices;
    auto get_invariant_mass(const std::vector<int>& indices) const -> double;
};

class DTFFitter {
public:
    explicit DTFFitter(const DecayDescriptor& descriptor);
    auto add_particle(const std::string& role, const Particle& particle) -> DTFFitter&;
    auto constrain_mass(double mass) -> DTFFitter&;
    auto constrain_vertex() -> DTFFitter&;
    auto constrain_pointing(const ROOT::Math::XYZVector& origin) -> DTFFitter&;
    auto fit() -> DTFResult;
};

class DecayTreeFitterService : public Service {
public:
    auto create_fitter(const DecayDescriptor& descriptor) -> std::unique_ptr<DTFFitter>;
    auto fit_two_body(const Particle& d1, const Particle& d2, double mother_mass,
                      const ROOT::Math::XYZVector& origin = {0,0,0}) -> DTFResult;
    auto fit_with_pointing(const std::vector<Particle>& daughters, double mother_mass,
                           const ROOT::Math::XYZVector& origin) -> DTFResult;
};
```
