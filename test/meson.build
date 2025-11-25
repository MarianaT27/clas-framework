# Test suite using Google Test

gtest = subproject('gtest')
gtest_dep = gtest.get_variable('gtest_dep')
gtest_main_dep = gtest.get_variable('gtest_main_dep')

test_inc = include_directories('../include')

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

# Register tests
test('Cutflow Tests', test_cutflow)
test('Service Tests', test_services)
