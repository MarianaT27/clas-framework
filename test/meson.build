# Test suite using Google Test

gtest = subproject('gtest')
gtest_dep = gtest.get_variable('gtest_dep')
gtest_main_dep = gtest.get_variable('gtest_main_dep')

# Common test dependencies
test_deps = [framework_dep, clas12_dep, algorithms_dep, gtest_dep, gtest_main_dep, fmt_dep, root_dep]

# Test executables
test_cutflow = executable(
  'test_cutflow',
  'test_cutflow.cpp',
  dependencies: test_deps,
)

test_services = executable(
  'test_services',
  'test_services.cpp',
  dependencies: test_deps,
)

# Unit tests for Result type
test_result = executable(
  'test_result',
  'test_result.cpp',
  dependencies: test_deps,
)

# Unit tests for ServiceHandle
test_service_handle = executable(
  'test_service_handle',
  'test_service_handle.cpp',
  dependencies: test_deps,
)

# Unit tests for Event store
test_event_store = executable(
  'test_event_store',
  'test_event_store.cpp',
  dependencies: test_deps,
)

# Unit tests for LoggingService
test_logging_service = executable(
  'test_logging_service',
  'test_logging_service.cpp',
  dependencies: test_deps,
)

# Unit tests for Application exception handling
test_application_exception = executable(
  'test_application_exception',
  'test_application_exception.cpp',
  dependencies: test_deps,
)

# Unit tests for Range Adaptors
test_ranges = executable(
  'test_ranges',
  'test_ranges.cpp',
  dependencies: test_deps,
)

# Register tests
test('Cutflow Tests', test_cutflow)
test('Service Tests', test_services)
test('Result Type Tests', test_result)
test('ServiceHandle Tests', test_service_handle)
test('Event Store Tests', test_event_store)
test('Logging Service Tests', test_logging_service)
test('Application Exception Tests', test_application_exception)
test('Range Adaptors Tests', test_ranges)

# Unit tests for SimpleAlgorithm
test_simple_algorithm = executable(
  'test_simple_algorithm',
  'test_simple_algorithm.cpp',
  dependencies: test_deps + [toml_dep],
)

test('SimpleAlgorithm Tests', test_simple_algorithm)

# Unit tests for Generator coroutine type
test_generator = executable(
  'test_generator',
  'test_generator.cpp',
  dependencies: test_deps,
)

test('Generator Tests', test_generator)

# Integration tests for event processing features
test_event_processing_integration = executable(
  'test_event_processing_integration',
  'test_event_processing_integration.cpp',
  dependencies: test_deps + [hipo4_dep],
)

test('Event Processing Integration Tests', test_event_processing_integration)

# Unit tests for Histogram Templates
test_histogram_templates = executable(
  'test_histogram_templates',
  'test_histogram_templates.cpp',
  dependencies: test_deps,
)

test('Histogram Templates Tests', test_histogram_templates)

# Unit tests for ConfigSchema
test_config_schema = executable(
  'test_config_schema',
  'test_config_schema.cpp',
  dependencies: test_deps + [toml_dep],
)

test('ConfigSchema Tests', test_config_schema)

# Concurrency regression test for IAlgorithm state and counters
test_algorithm_concurrency = executable(
  'test_algorithm_concurrency',
  'test_algorithm_concurrency.cpp',
  dependencies: test_deps,
)

test('Algorithm Concurrency Tests', test_algorithm_concurrency)

# Smoke test for generated bank wrappers + PindexLinker
test_bankdefs_compile = executable(
  'test_bankdefs_compile',
  'test_bankdefs_compile.cpp',
  dependencies: test_deps,
)

test('BankDefs Compile Tests', test_bankdefs_compile)

# Unit tests for column-oriented vectorised fill (Event::column<T> +
# ThreadLocalHistogram::fill_span).
test_column_fill = executable(
  'test_column_fill',
  'test_column_fill.cpp',
  dependencies: test_deps,
)

test('Column Fill Tests', test_column_fill)

# Concurrency + Accumulator concept tests for the per-worker
# ThreadLocalHistogram.
test_thread_local_histogram = executable(
  'test_thread_local_histogram',
  'test_thread_local_histogram.cpp',
  dependencies: test_deps,
)

test('ThreadLocalHistogram Tests', test_thread_local_histogram)
