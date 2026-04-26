# Contributing

Thanks for working on clas-framework. This page collects the conventions
that are not obvious from reading the code.

## Build and test

```bash
meson setup build
meson compile -C build
meson test -C build
```

CI compiles with `-Dstrict_warnings=true -Werror` and runs the suite under
`-Db_sanitize=address,undefined`. Reproduce locally before opening a merge
request:

```bash
meson setup build-strict -Dstrict_warnings=true
meson compile -C build-strict

meson setup build-asan --buildtype=debug -Db_sanitize=address,undefined
meson test -C build-asan
```

## Code style

- Format with `clang-format` (config in `.clang-format`).
- Lint with `clang-tidy` (config in `.clang-tidy`).
- Editor settings in `.editorconfig` — turn on EditorConfig in your editor.
- 4-space indent, LF endings, UTF-8.
- C++20. Coroutines need GCC 12+ or Clang 14+.

Run before pushing:

```bash
git ls-files '*.cpp' '*.hpp' | xargs clang-format -i
```

## Dependency injection in algorithms

Use `ServiceRef<T>` for service members. Example: `ElectronAnalysisAlg`
is the canonical pattern.

```cpp
class MyAlgorithm : public clas12::framework::Algorithm {
public:
    explicit MyAlgorithm(std::string name = "MyAlgorithm")
        : Algorithm(std::move(name)) {
        required_services(m_hist, m_electron_cuts);
    }

private:
    clas12::framework::ServiceRef<clas12::framework::HistogramService> m_hist;
    clas12::framework::ServiceRef<clas12::framework::ElectronCutsService> m_electron_cuts;
};
```

The framework injects them before `on_initialize()` runs and aborts with a
clear error if a dependency is missing. Do not look services up by string
name in `on_initialize()` — that is the legacy pattern.

## Histograms

For per-event filling on multiple worker threads, use
`ThreadLocalHistogram` / `ThreadLocalHistogramGroup`. Each worker gets its
own copy and the framework merges them at finalize. Do not introduce CAS
or mutex-based atomic histograms — that path was removed.

## Adding a new algorithm or service

Templates live in `templates/`. Copy one and adapt. For services,
register in `framework/src/ApplicationBuilder.cpp`
(`with_standard_services`) so it is wired up by default.

## Adding a test

Tests live in `test/` and use Google Test. Register in `test/meson.build`
following the existing pattern. Property-based tests (rapidcheck) end in
`_properties.cpp`.

If you add tests for a service that has dependencies declared via
`require_service<T>()`, include both `<clas12/framework/core/ServiceManager.hpp>`
and the dependency's header in the test source — the template
instantiation needs both definitions to link.

## Pull request checklist

- [ ] `meson test -C build` passes.
- [ ] `clang-format` clean (`git diff` shows no whitespace-only changes).
- [ ] Strict-warnings build clean (`-Dstrict_warnings=true`).
- [ ] Tests added or updated for new behaviour.
- [ ] No new TODO/FIXME comments without an issue link.
- [ ] CHANGELOG entry if user-facing.
