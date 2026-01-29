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
                     int nbins, double xmin, double xmax) -> AtomicHistogram1D*;
    auto book_hist2D(const std::string& path, const std::string& title,
                     int nbinsx, double xmin, double xmax,
                     int nbinsy, double ymin, double ymax) -> AtomicHistogram2D*;

    // Physics histogram templates
    auto book_momentum_hist(...) -> AtomicHistogram1D*;
    auto book_theta_hist(...) -> AtomicHistogram1D*;
    auto book_phi_hist(...) -> AtomicHistogram1D*;
    auto book_q2_hist(...) -> AtomicHistogram1D*;
    auto book_w_hist(...) -> AtomicHistogram1D*;
    auto book_xb_hist(...) -> AtomicHistogram1D*;
    auto book_vz_hist(...) -> AtomicHistogram1D*;
    auto book_theta_vs_p(...) -> AtomicHistogram2D*;

    // Access methods
    auto get_atomic_hist1D(const std::string& path) -> AtomicHistogram1D*;
    auto get_atomic_hist2D(const std::string& path) -> AtomicHistogram2D*;
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

### MCTruthService

MC truth matching and efficiency studies.

```cpp
class MCTruthService : public Service {
public:
    auto load_mc_particles(hipo::bank& mc_bank) -> void;
    auto mc_particles() const -> const std::vector<MCParticle>&;
    auto mc_particles(PIDs pid) const -> std::vector<const MCParticle*>;
    auto match(const Particle& rec, hipo::bank& rec_match_bank) -> const MCParticle*;
    auto momentum_resolution(const Particle& rec) const -> std::optional<double>;
    auto is_reconstructed(const MCParticle& mc) const -> bool;
};
```

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

### GoodRunListService

Data quality filtering.

```cpp
class GoodRunListService : public Service {
public:
    auto load_from_file(const std::string& grl_path) -> StatusCode;
    auto is_good_run(int run_number) const -> bool;
    auto get_run_info(int run_number) const -> std::optional<RunInfo>;
    auto require_flag(const std::string& flag) -> GoodRunListService&;
    auto set_run_range(int min_run, int max_run) -> GoodRunListService&;
    auto total_luminosity() const -> double;
    auto luminosity_weight(int run_number) const -> double;
    auto check_event(const Event& event) const -> bool;
};
```

### EventMixingService

Combinatorial background estimation.

```cpp
class EventMixingService : public Service {
public:
    auto add_vz_bins(const std::vector<double>& edges) -> void;
    auto add_Q2_bins(const std::vector<double>& edges) -> void;
    template<typename T>
    auto add_to_pool(const std::string& pool_name, const Event& event,
                     const std::vector<T>& particles) -> void;
    template<typename T>
    auto get_mixed_particles(const std::string& pool_name, const Event& current_event)
        -> std::vector<const T*>;
    template<typename T1, typename T2>
    auto get_mixed_pairs(const std::string& pool1, const std::string& pool2,
                         const Event& current_event) -> std::vector<std::pair<const T1*, const T2*>>;
    auto mixed_event_weight() const -> double;
};
```

### SystematicService

Systematic uncertainty framework.

```cpp
class SystematicService : public Service {
public:
    auto add_energy_scale(double sigma = 0.01) -> void;
    auto add_momentum_resolution(double sigma = 0.005) -> void;
    auto add_efficiency_variation(const std::string& name,
                                  std::function<double(const Event&)> func) -> void;
    auto set_variation(const std::string& name) -> void;
    auto current_variation() const -> const std::string&;
    auto vary(const Particle& p) const -> Particle;
    auto weight(const Event& event) const -> double;
};
```

### MVAService

Machine learning classification.

```cpp
class MVAService : public Service {
public:
    auto register_model(const std::string& name, const std::string& weights_path) -> bool;
    auto evaluate(const std::string& model_name, const Particle& particle) -> float;
    auto passes_cut(const std::string& model_name, const Particle& particle, float threshold) -> bool;
};
```

### WeightService

Event weights for corrections.

```cpp
enum class WeightType { Luminosity, Acceptance, Efficiency, Trigger, PID, Radiative, Custom };

class WeightService : public Service {
public:
    auto register_weight(const std::string& name, WeightType type,
                         std::function<double(const Event&)> calculator) -> void;
    auto load_efficiency_map(const std::string& name, const std::string& root_file,
                             const std::string& hist_name) -> StatusCode;
    auto get_weight(const std::string& name, const Event& event) const -> EventWeight;
    auto get_total_weight(const Event& event) const -> EventWeight;
    auto lookup_efficiency(const std::string& map_name, double x, double y) const -> double;
};
```

---

## Advanced Fitting Services

### VertexFittingService

Primary/secondary vertex reconstruction.

```cpp
struct VertexFitResult {
    bool converged;
    ROOT::Math::XYZVector position;
    Eigen::Matrix3d covariance;
    double chi2, probability;
    int ndf;
    std::vector<ROOT::Math::PxPyPzEVector> refitted_momenta;
    auto is_good(double min_prob = 0.01) const -> bool;
    auto distance_from(const ROOT::Math::XYZVector& point) const -> double;
    auto significance_from(const ROOT::Math::XYZVector& point) const -> double;
};

class VertexFittingService : public Service {
public:
    auto fit_primary_vertex(const std::vector<Particle>& tracks,
                           VertexConstraint constraint = VertexConstraint::None) -> VertexFitResult;
    auto fit_primary_vertex_robust(const std::vector<Particle>& tracks,
                                   double outlier_chi2_cut = 9.0) -> VertexFitResult;
    auto fit_secondary_vertex(const Particle& track1, const Particle& track2) -> VertexFitResult;
    auto fit_secondary_vertex_with_mass(const std::vector<Particle>& tracks,
                                        double mass_constraint) -> VertexFitResult;
    auto flight_distance(const VertexFitResult& primary,
                        const VertexFitResult& secondary) const -> std::pair<double, double>;
    auto decay_length_significance(const VertexFitResult& primary,
                                   const VertexFitResult& secondary,
                                   const ROOT::Math::XYZVector& momentum) const -> double;
};
```

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

### PIDService

Log-likelihood particle identification.

```cpp
enum class PIDHypothesis { Electron, Positron, Muon, Pion, Kaon, Proton, Deuteron, Unknown };

struct PIDResult {
    PIDHypothesis best_hypothesis;
    double confidence;
    std::map<PIDHypothesis, double> log_likelihoods;
    std::map<PIDHypothesis, double> delta_ll;
    auto get_dll(PIDHypothesis h1, PIDHypothesis h2) const -> double;
    auto probability(PIDHypothesis h) const -> double;
};

class PIDService : public Service {
public:
    Property<bool> m_use_tof{"use_tof", true};
    Property<bool> m_use_cherenkov{"use_cherenkov", true};
    auto compute_pid(const Particle& particle, Event& event) -> PIDResult;
    auto delta_log_likelihood(const Particle& p, Event& e,
                              PIDHypothesis h1, PIDHypothesis h2) -> double;
    auto is_electron/is_kaon/is_proton(const Particle& p, Event& e, double dll_cut) -> bool;
    auto efficiency_weight(const Particle& p, PIDHypothesis h, double dll_cut) -> double;
};
```

### TrackQualityService

Track quality assessment.

```cpp
enum class TrackQuality { VeryLoose, Loose, Standard, Tight, VeryTight };

struct TrackQualityInfo {
    int ndf;
    double chi2, chi2_ndf;
    int n_superlayers;
    std::array<double, 3> dc_edge;
    int sector;
    bool has_ftof, has_htcc, has_ecal;
    auto quality_score() const -> double;
};

class TrackQualityService : public Service {
public:
    Property<double> m_chi2_cut_standard{"chi2_cut_standard", 10.0};
    Property<int> m_min_ndf{"min_ndf", 5};
    Property<double> m_dc_edge_cut{"dc_edge_cut", 5.0};
    auto assess_quality(const Particle& track, Event& event) -> TrackQualityInfo;
    auto passes_quality(const Particle& track, Event& event,
                        TrackQuality level = TrackQuality::Standard) -> bool;
    auto filter_by_quality(const std::vector<Particle>& tracks, Event& event,
                           TrackQuality level) -> std::vector<Particle>;
    auto best_track(const std::vector<Particle>& tracks, Event& event) -> std::optional<Particle>;
};
```

---

## Cross-Section Services

### sPlotService

Statistical background subtraction using sWeights.

```cpp
struct sPlotResult {
    bool success;
    std::map<std::string, double> yields, yield_errors;
    std::map<std::string, std::vector<double>> sweights;
    Eigen::MatrixXd covariance;
    auto signal_yield() const -> double;
    auto get_sweight(const std::string& component, size_t event_idx) const -> double;
};

class sPlotService : public Service {
public:
    auto create_splot(const std::string& name, const std::string& var,
                      double var_min, double var_max) -> sPlotService&;
    auto add_gaussian_signal(const std::string& splot, const std::string& name,
                             double mean, double sigma) -> sPlotService&;
    auto add_crystal_ball_signal(const std::string& splot, const std::string& name,
                                 double mean, double sigma, double alpha, double n) -> sPlotService&;
    auto add_polynomial_background(const std::string& splot, const std::string& name,
                                   int order) -> sPlotService&;
    auto compute_sweights(const std::string& splot, const std::vector<double>& data) -> sPlotResult;
    auto fill_with_sweights(const std::string& splot, const std::string& component,
                            TH1D* histogram, const std::vector<double>& values) -> void;
};
```

### UnfoldingService

Detector response correction.

```cpp
enum class UnfoldingMethod { MatrixInversion, BinByBin, SVD, Bayesian };

struct UnfoldingResult {
    bool success;
    std::unique_ptr<TH1D> unfolded, refolded;
    Eigen::MatrixXd covariance;
    double chi2_refolded;
};

class UnfoldingService : public Service {
public:
    Property<int> m_bayesian_iterations{"bayesian_iterations", 4};
    auto build_response(const std::string& name, const std::vector<double>& true_values,
                        const std::vector<double>& reco_values, int nbins_true, int nbins_reco,
                        double range_min, double range_max) -> UnfoldingService&;
    auto unfold_bayesian(const std::string& name, const TH1D* measured, int n_iterations) -> UnfoldingResult;
    auto unfold_svd(const std::string& name, const TH1D* measured, int k_reg) -> UnfoldingResult;
    auto unfold_bin_by_bin(const std::string& name, const TH1D* measured) -> UnfoldingResult;
    auto get_purity/stability/efficiency(const std::string& name) const -> std::unique_ptr<TH1D>;
    auto closure_test(const std::string& name, const TH1D* mc_true, UnfoldingMethod method) -> UnfoldingResult;
};
```

### CrossSectionService

Cross-section calculation.

```cpp
struct CrossSectionResult {
    std::string name, variable, units;
    std::vector<double> bin_centers, bin_widths;
    std::vector<double> cross_sections, stat_errors;
    std::vector<double> syst_errors_up, syst_errors_down;
    double total_cross_section, integrated_luminosity;
    auto to_graph() const -> std::unique_ptr<TGraphAsymmErrors>;
    auto to_histogram() const -> std::unique_ptr<TH1D>;
    void print_summary() const;
};

class CrossSectionService : public Service {
public:
    Property<double> m_beam_energy{"beam_energy", 10.6};
    auto define_measurement(const std::string& name, const std::string& variable,
                            const std::string& units = "nb") -> CrossSectionService&;
    auto set_luminosity(const std::string& name, double luminosity) -> CrossSectionService&;
    auto set_yield(const std::string& name, const TH1D* yield) -> CrossSectionService&;
    auto set_acceptance(const std::string& name, const TH1D* acceptance) -> CrossSectionService&;
    auto calculate_acceptance(const std::string& name, const TH1D* reco_mc,
                              const TH1D* gen_mc) -> CrossSectionService&;
    auto set_efficiency(const std::string& name, const TH1D* efficiency) -> CrossSectionService&;
    auto set_radiative_correction(const std::string& name, const TH1D* rad_corr) -> CrossSectionService&;
    auto calculate(const std::string& name) -> CrossSectionResult;
    auto calculate_with_systematics(const std::string& name) -> CrossSectionResult;
};
```
