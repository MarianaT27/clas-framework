# Test suite using Google Test and RapidCheck

gtest = subproject('gtest')
gtest_dep = gtest.get_variable('gtest_dep')
gtest_main_dep = gtest.get_variable('gtest_main_dep')

# RapidCheck for property-based testing
rapidcheck_proj = subproject('rapidcheck')
rapidcheck_dep = rapidcheck_proj.get_variable('rapidcheck_dep')

test_inc = include_directories('../include')
hipo_inc = include_directories('..')

# Test executables
test_cutflow = executable(
  'test_cutflow',
  'test_cutflow.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

test_services = executable(
  'test_services',
  'test_services.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

# Property-based tests for StatusCode
test_statuscode_properties = executable(
  'test_statuscode_properties',
  'test_statuscode_properties.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, rapidcheck_dep, fmt_dep],
)

# Unit tests for Result type
test_result = executable(
  'test_result',
  'test_result.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

# Property-based tests for ConfigService
test_config_properties = executable(
  'test_config_properties',
  'test_config_properties.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, rapidcheck_dep, fmt_dep, toml_dep],
)

# Unit tests for ServiceHandle
test_service_handle = executable(
  'test_service_handle',
  'test_service_handle.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

# Property-based tests for Service Dependency Ordering
test_service_dependency_properties = executable(
  'test_service_dependency_properties',
  'test_service_dependency_properties.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, rapidcheck_dep, fmt_dep],
)

# Unit tests for Event store
test_event_store = executable(
  'test_event_store',
  'test_event_store.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

# Property-based tests for Event store
test_event_store_properties = executable(
  'test_event_store_properties',
  'test_event_store_properties.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, rapidcheck_dep, fmt_dep],
)

# Unit tests for LoggingService
test_logging_service = executable(
  'test_logging_service',
  'test_logging_service.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

# Property-based tests for LoggingService
test_logging_properties = executable(
  'test_logging_properties',
  'test_logging_properties.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, rapidcheck_dep, fmt_dep],
)

# Unit tests for Application exception handling
test_application_exception = executable(
  'test_application_exception',
  'test_application_exception.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

# Property-based tests for Pi0 Reconstruction
test_pi0_reconstruction_properties = executable(
  'test_pi0_reconstruction_properties',
  'test_pi0_reconstruction_properties.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, rapidcheck_dep, fmt_dep, root_dep],
)

# Unit tests for Range Adaptors
test_ranges = executable(
  'test_ranges',
  'test_ranges.cpp',
  '../src/CLAS/Particle.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep, root_dep],
)

# Register tests
test('Cutflow Tests', test_cutflow)
test('Service Tests', test_services)
test('StatusCode Property Tests', test_statuscode_properties)
test('Result Type Tests', test_result)
test('ConfigService Property Tests', test_config_properties)
test('ServiceHandle Tests', test_service_handle)
test('Service Dependency Property Tests', test_service_dependency_properties)
test('Event Store Tests', test_event_store)
test('Event Store Property Tests', test_event_store_properties)
test('Logging Service Tests', test_logging_service)
test('Logging Property Tests', test_logging_properties)
test('Application Exception Tests', test_application_exception)
test('Pi0 Reconstruction Property Tests', test_pi0_reconstruction_properties)
test('Range Adaptors Tests', test_ranges)

# Unit tests for SimpleAlgorithm
test_simple_algorithm = executable(
  'test_simple_algorithm',
  'test_simple_algorithm.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep, toml_dep],
)

test('SimpleAlgorithm Tests', test_simple_algorithm)

# Unit tests for Generator coroutine type
test_generator = executable(
  'test_generator',
  'test_generator.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

test('Generator Tests', test_generator)

# Integration tests for event processing features
test_event_processing_integration = executable(
  'test_event_processing_integration',
  'test_event_processing_integration.cpp',
  '../src/CLAS/Particle.cpp',
  include_directories: [test_inc, hipo_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep, hipo_dep, root_dep],
)

test('Event Processing Integration Tests', test_event_processing_integration)

# Unit tests for Histogram Templates
test_histogram_templates = executable(
  'test_histogram_templates',
  'test_histogram_templates.cpp',
  '../src/Framework/Histogram.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep, root_dep],
)

test('Histogram Templates Tests', test_histogram_templates)

# Unit tests for CutBuilder
test_cutbuilder = executable(
  'test_cutbuilder',
  'test_cutbuilder.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

test('CutBuilder Tests', test_cutbuilder)

# Unit tests for ConfigSchema
test_config_schema = executable(
  'test_config_schema',
  'test_config_schema.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep, toml_dep],
)

test('ConfigSchema Tests', test_config_schema)

# Unit tests for Inject and InjectableAlgorithm
test_inject = executable(
  'test_inject',
  'test_inject.cpp',
  include_directories: [test_inc],
  dependencies: [gtest_dep, gtest_main_dep, fmt_dep],
)

test('Inject Tests', test_inject)
