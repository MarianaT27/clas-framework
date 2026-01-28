# Services

Framework services for shared functionality.

## HistogramService

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
    auto book_momentum_hist(const std::string& path, const std::string& title,
                            int nbins = 100, double pmin = 0, double pmax = 12) -> AtomicHistogram1D*;
    auto book_theta_hist(const std::string& path, const std::string& title,
                         int nbins = 180) -> AtomicHistogram1D*;
    auto book_phi_hist(const std::string& path, const std::string& title,
                       int nbins = 360) -> AtomicHistogram1D*;
    auto book_q2_hist(const std::string& path, const std::string& title,
                      int nbins = 100, double q2min = 0, double q2max = 12) -> AtomicHistogram1D*;
    auto book_w_hist(const std::string& path, const std::string& title,
                     int nbins = 100, double wmin = 0, double wmax = 5) -> AtomicHistogram1D*;
    auto book_xb_hist(const std::string& path, const std::string& title,
                      int nbins = 100) -> AtomicHistogram1D*;
    auto book_vz_hist(const std::string& path, const std::string& title,
                      int nbins = 200, double zmin = -40, double zmax = 40) -> AtomicHistogram1D*;

    // 2D physics templates
    auto book_theta_vs_p(const std::string& path, const std::string& title) -> AtomicHistogram2D*;

    // Access methods
    auto get_atomic_hist1D(const std::string& path) -> AtomicHistogram1D*;
    auto get_atomic_hist2D(const std::string& path) -> AtomicHistogram2D*;
    auto get_hist1D(const std::string& path) -> TH1D*;  // Returns ROOT version for fitting/export
    auto get_hist2D(const std::string& path) -> TH2D*;  // Returns ROOT version for fitting/export
};

// AtomicHistogram1D - thread-safe lock-free histogram
class AtomicHistogram1D {
public:
    auto fill(double value) -> void;
    auto fill(double value, double weight) -> void;
    auto entries() const -> size_t;
    auto to_root(const std::string& name, const std::string& title) const -> std::unique_ptr<TH1D>;
};

// AtomicHistogram2D - thread-safe lock-free 2D histogram
class AtomicHistogram2D {
public:
    auto fill(double x, double y) -> void;
    auto fill(double x, double y, double weight) -> void;
    auto entries() const -> size_t;
    auto to_root(const std::string& name, const std::string& title) const -> std::unique_ptr<TH2D>;
};
```

## LoggingService

```cpp
class LoggingService : public IService {
public:
    auto set_log_level(LogLevel level) -> void;
    auto set_log_file(const std::string& filename) -> void;
    
    auto debug(const std::string& source, const std::string& msg) -> void;
    auto info(const std::string& source, const std::string& msg) -> void;
    auto warning(const std::string& source, const std::string& msg) -> void;
    auto error(const std::string& source, const std::string& msg) -> void;
};

// Global singleton
class Log {
public:
    static auto info(const std::string& source, const std::string& fmt, Args&&... args) -> void;
    static auto debug(...) -> void;
    static auto warning(...) -> void;
    static auto error(...) -> void;
};
```

## NTupleService

```cpp
class NTupleService : public IService {
public:
    auto set_output_file(const std::string& filename) -> void;
    
    template<typename T>
    auto register_branch(const std::string& tree, const std::string& branch, T* ptr) -> void;
    
    auto fill(const std::string& tree) -> void;
};
```

## ConditionsService

```cpp
class ConditionsService : public IService {
public:
    auto set_run_number(int run) -> void;
    auto get_time_offset(const std::string& detector, int sector) -> double;
    
    template<typename T>
    auto set_value(const std::string& key, T value, IntervalOfValidity iov) -> void;
};
```
