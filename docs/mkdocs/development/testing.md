# Testing

Unit testing with Google Test.

## Running Tests

```bash
# Run all tests
meson test -C build

# Run specific test
meson test -C build test_cutflow

# Verbose output
meson test -C build -v
```

## Writing Tests

Tests are in the `test/` directory:

```cpp
#include <gtest/gtest.h>
#include "Framework/Cutflow.hpp"

TEST(CutflowTest, AddCut) {
    Framework::Cutflow cutflow("Test");
    cutflow.add_cut("Cut1");
    cutflow.add_cut("Cut2");
    
    EXPECT_EQ(cutflow.get_count("Cut1"), 0);
}

TEST(CutflowTest, PassCut) {
    Framework::Cutflow cutflow("Test");
    cutflow.add_cut("Cut1");
    
    cutflow.pass("Cut1");
    cutflow.pass("Cut1");
    
    EXPECT_EQ(cutflow.get_count("Cut1"), 2);
}

TEST(CutflowTest, Efficiency) {
    Framework::Cutflow cutflow("Test");
    cutflow.add_cut("All");
    cutflow.add_cut("Selected");
    
    for (int i = 0; i < 100; i++) {
        cutflow.pass("All");
        if (i < 50) cutflow.pass("Selected");
    }
    
    EXPECT_NEAR(cutflow.get_efficiency("Selected"), 0.5, 0.01);
}
```

## Test Categories

| Category | Description |
|----------|-------------|
| Unit tests | Test individual classes |
| Integration tests | Test component interactions |
| Physics tests | Validate physics calculations |

## Adding Tests

1. Create `test/test_myfeature.cpp`
2. Add to `test/meson.build`:

```meson
test('test_myfeature',
  executable('test_myfeature',
    'test_myfeature.cpp',
    dependencies: [gtest_dep, framework_dep],
  )
)
```

## Best Practices

- Test edge cases
- Use descriptive test names
- One assertion per test when possible
- Mock external dependencies
