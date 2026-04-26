# Code Style

Coding conventions for the CLAS12 Analysis Framework.

## General

- C++20 standard
- 4-space indentation
- 100 character line limit
- Use `.clang-format` for automatic formatting

## Naming

| Type | Convention | Example |
|------|------------|---------|
| Classes | PascalCase | `HistogramService` |
| Functions | snake_case | `book_hist1D()` |
| Variables | snake_case | `electron_momentum` |
| Member variables | m_ prefix | `m_hist_svc` |
| Constants | UPPER_CASE | `ELECTRON_MASS` |
| Namespaces | PascalCase | `Framework` |

## Functions

```cpp
// Use trailing return type
auto calculate_momentum(const Particle& p) -> double {
    return std::sqrt(p.px()*p.px() + p.py()*p.py() + p.pz()*p.pz());
}

// Use const references for input
auto process(const std::vector<Particle>& particles) -> void;

// Use unique_ptr for ownership transfer
auto create_algorithm() -> std::unique_ptr<Algorithm>;
```

## Classes

```cpp
class MyClass {
public:
    // Constructors first
    explicit MyClass(std::string name);
    
    // Public methods
    auto do_something() -> void;
    
    // Getters (const)
    auto name() const -> const std::string&;

private:
    // Member variables with m_ prefix
    std::string m_name;
    int m_count = 0;
};
```

## Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Calculate the invariant mass of two particles
 * @param p1 First particle
 * @param p2 Second particle
 * @return Invariant mass in GeV
 */
auto invariant_mass(const Particle& p1, const Particle& p2) -> double;
```

## Includes

Order:

1. Corresponding header
2. Project headers
3. Third-party headers
4. Standard library

```cpp
#include "MyClass.hpp"

#include "Framework/Event.hpp"
#include "CLAS/Particle.hpp"

#include <fmt/format.h>
#include <ROOT/TH1D.h>

#include <vector>
#include <string>
```
