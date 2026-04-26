# Example 01: Simple Algorithm

This example demonstrates the basic `Algorithm` base class.

## Key Concepts

- **Hook pattern**: derive from `Algorithm` and override `on_initialize()`, `process()`, and `on_finalize()`.
- **Service injection**: declare dependencies with `required_services(...)` in the constructor.
- **Properties**: declare configurable parameters with `Property<T>` so they can be set from the analysis TOML.
- **Single CLI flag**: every binary takes `--config <path.toml>` only.

## Running

```bash
./build/example_simple_algorithm --config config/example_simple.toml
```

## Code Structure

```cpp
class MyAlgorithm : public clas12::framework::Algorithm<MyAlgorithm> {
    // Constructor: declare properties
    // configure(): declare services
    // process(): per-event logic
};
```
