project('rapidcheck', 'cpp',
  version: '0.0.0',
  default_options: ['cpp_std=c++17', 'warning_level=0']
)

rapidcheck_sources = files(
  'src/Log.cpp',
  'src/Random.cpp',
  'src/GenerationFailure.cpp',
  'src/BeforeMinimalTestCase.cpp',
  'src/gen/Text.cpp',
  'src/gen/Numeric.cpp',
  'src/gen/detail/ScaleInteger.cpp',
  'src/gen/detail/Recipe.cpp',
  'src/gen/detail/ExecHandler.cpp',
  'src/gen/detail/GenerationHandler.cpp',
  'src/Show.cpp',
  'src/detail/StringSerialization.cpp',
  'src/detail/ParseException.cpp',
  'src/detail/ImplicitParam.cpp',
  'src/detail/MulticastTestListener.cpp',
  'src/detail/FrequencyMap.cpp',
  'src/detail/Base64.cpp',
  'src/detail/Property.cpp',
  'src/detail/Any.cpp',
  'src/detail/Assertions.cpp',
  'src/detail/LogTestListener.cpp',
  'src/detail/DefaultTestListener.cpp',
  'src/detail/TestParams.cpp',
  'src/detail/Serialization.cpp',
  'src/detail/ReproduceListener.cpp',
  'src/detail/PropertyContext.cpp',
  'src/detail/Configuration.cpp',
  'src/detail/Platform.cpp',
  'src/detail/MapParser.cpp',
  'src/detail/Testing.cpp',
  'src/detail/TestMetadata.cpp',
  'src/detail/Results.cpp',
  'src/Classify.cpp',
  'src/Check.cpp',
)

rapidcheck_inc = include_directories('include', 'extras/gtest/include')

rapidcheck_lib = static_library(
  'rapidcheck',
  rapidcheck_sources,
  include_directories: rapidcheck_inc,
)

rapidcheck_dep = declare_dependency(
  include_directories: rapidcheck_inc,
  link_with: rapidcheck_lib,
)
