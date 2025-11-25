# Python bindings

py = import('python').find_installation('python3')

pybind11 = subproject('pybind11')
pybind11_dep = pybind11.get_variable('pybind11_dep')

py.extension_module(
  'pyclas12',
  'pyclas12.cpp',
  include_directories: [inc],
  dependencies: [pybind11_dep, fmt_dep, hipo_dep, root_dep],
  install: true,
)
