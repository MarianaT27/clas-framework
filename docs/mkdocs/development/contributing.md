# Contributing

Guidelines for contributing to the CLAS12 Analysis Framework.

## Getting Started

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make your changes
4. Run tests: `meson test -C build`
5. Submit a merge request

## Code Style

See [Code Style](code-style.md) for detailed guidelines.

Key points:

- Use C++20 features
- Follow the existing naming conventions
- Add Doxygen comments to public APIs
- Write unit tests for new functionality

## Pull Request Process

1. Update documentation if needed
2. Add tests for new features
3. Ensure all tests pass
4. Update CHANGELOG if applicable
5. Request review from maintainers

## Reporting Issues

When reporting bugs, include:

- Framework version
- Compiler and ROOT version
- Minimal reproducing example
- Expected vs actual behavior

## Adding New Algorithms

1. Create header in `include/Algorithms/`
2. Implement in `src/Algorithms/` (if needed)
3. Add to `meson.build`
4. Write unit tests
5. Add documentation

## Adding New Services

1. Implement `IService` interface
2. Add to `include/Framework/Services/`
3. Register in `ServiceManager`
4. Write unit tests
5. Document usage
