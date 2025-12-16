# Services

Framework services for shared functionality.

## HistogramService

```cpp
class HistogramService : public IService {
public:
    auto set_output_file(const std::string& filename) -> void;
    
    auto book_hist1D(const std::string& path, const std::string& title,
                     int nbins, double xmin, double xmax) -> TH1D*;
    
    auto book_hist2D(const std::string& path, const std::string& title,
                     int nbinsx, double xmin, double xmax,
                     int nbinsy, double ymin, double ymax) -> TH2D*;
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
