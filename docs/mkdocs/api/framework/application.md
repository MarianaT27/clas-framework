# Application

The main application class for running analyses.

## Class Definition

```cpp
namespace Framework {

class Application {
public:
    Application(std::string name);
    
    auto add_algorithm(std::unique_ptr<IAlgorithm> algorithm) -> void;
    auto service_manager() -> ServiceManager&;
    auto set_input_files(std::vector<std::string> files) -> void;
    auto set_percentage(double percentage) -> void;
    auto set_num_threads(int num_threads) -> void;
    auto set_logger(LoggingService* log) -> void;
    
    auto initialize() -> StatusCode;
    auto run() -> StatusCode;
    auto finalize() -> StatusCode;
    
    auto exception_count() const -> long;
};

}
```

## Methods

### Constructor

```cpp
Application(std::string name)
```

Creates a new application with the given name.

### add_algorithm

```cpp
auto add_algorithm(std::unique_ptr<IAlgorithm> algorithm) -> void
```

Adds an algorithm to the processing sequence. Algorithms execute in the order they are added.

### service_manager

```cpp
auto service_manager() -> ServiceManager&
```

Returns a reference to the service manager for adding and retrieving services.

### set_input_files

```cpp
auto set_input_files(std::vector<std::string> files) -> void
```

Sets the input HIPO files to process.

### set_num_threads

```cpp
auto set_num_threads(int num_threads) -> void
```

Sets the number of threads for parallel event processing.

### initialize / run / finalize

```cpp
auto initialize() -> StatusCode
auto run() -> StatusCode
auto finalize() -> StatusCode
```

Lifecycle methods. Call in order: initialize → run → finalize.

## Example

```cpp
Framework::Application app("MyAnalysis");

// Add services
auto hist_svc = std::make_unique<Framework::HistogramService>();
hist_svc->set_output_file("output.root");
app.service_manager().add_service(std::move(hist_svc));

// Add algorithms
app.add_algorithm(std::make_unique<MyAlgorithm>());

// Configure
app.set_input_files({"data.hipo"});
app.set_num_threads(4);

// Run
app.initialize();
app.run();
app.finalize();
```
