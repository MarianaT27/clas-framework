# Example 01: Simple Algorithm

This example demonstrates the basic CRTP-based `SimpleAlgorithm` pattern.

## Key Concepts

- **CRTP Pattern**: Inherit from `SimpleAlgorithm<YourClass>` for reduced boilerplate
- **Service Injection**: Use `require_service<T>()` in `configure()` to declare dependencies
- **Properties**: Declare configurable parameters with `Property<T>`
- **Lifecycle**: Override `on_initialize()`, `process()`, and `on_finalize()`

## Running

```bash
./build/example_simple_algorithm data/sample.hipo
```

## Code Structure

```cpp
class MyAlgorithm : public clas12::framework::SimpleAlgorithm<MyAlgorithm> {
    // Constructor: declare properties
    // configure(): declare services
    // process(): per-event logic
};
```
