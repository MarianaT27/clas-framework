# Analysis Template

This template provides a starting point for creating new CLAS12 analysis executables.

## Features

- **Command-line argument parsing** with cxxopts
- **Standard service setup** (Logging, Histogram, Output)
- **Configuration file support** (TOML)
- **Builder pattern integration**

## Usage

### Using the scaffolding script (Recommended)

```bash
# Create a basic analysis
python templates/create_analysis.py --name my_analysis

# Create with a custom algorithm
python templates/create_analysis.py --name my_analysis --with-algorithm

# Specify custom namespace
python templates/create_analysis.py --name my_analysis --namespace physics
```

### After generation

1. Add your analysis to `analyses/meson.build`:
   ```meson
   subdir('my_analysis')
   ```

2. Rebuild:
   ```bash
   meson compile -C build
   ```

3. Run your analysis:
   ```bash
   ./build/my_analysis input.hipo -o output.root
   ```

## Template Variables

| Variable | Description | Example |
|----------|-------------|---------|
| `${ANALYSIS_NAME}` | Executable name (snake_case) | `my_analysis` |
| `${CLASS_NAME}` | Class name (PascalCase) | `MyAnalysis` |
| `${FILENAME}` | Source file name | `my_analysis.cpp` |
| `${DESCRIPTION}` | Brief description | `My custom analysis` |
| `${OUTPUT_FILE}` | Default output filename | `my_analysis_output.root` |
| `${NAMESPACE}` | C++ namespace | `myanalysis` |

## Command-Line Options

The generated analysis supports these options:

```
Usage: my_analysis [OPTIONS] <input_files...>

Options:
  -h, --help           Show help
  -i, --input FILES    Input HIPO files
  -o, --output FILE    Output ROOT file (default: my_analysis_output.root)
  -n, --nevents N      Number of events (-1 for all)
  -c, --config FILE    Configuration file (TOML)
```

## Directory Structure

After generation:
```
analyses/my_analysis/
├── my_analysis.cpp      # Main executable
├── MyAnalysisAlg.hpp    # Custom algorithm (if --with-algorithm)
└── meson.build          # Build configuration
```
