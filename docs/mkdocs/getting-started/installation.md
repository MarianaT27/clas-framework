# Installation

## Prerequisites

Before installing the CLAS12 Analysis Framework, ensure you have the following:

### Required

| Dependency | Version | Description |
|------------|---------|-------------|
| C++ Compiler | GCC 10+, Clang 12+ | C++20 support required |
| ROOT | 6.x | CERN ROOT for histograms and trees |
| Meson | 0.60+ | Build system |
| Ninja | 1.10+ | Build backend |

### Optional

| Dependency | Description |
|------------|-------------|
| Python 3 | For Python bindings and scripts |
| Doxygen | For generating API documentation |
| MkDocs | For building this documentation |

## Installation Steps

### 1. Clone the Repository

```bash
git clone https://gitlab.com/MathieuOuillon/clas-analysis clas12-framework
cd clas12-framework
```

### 2. Configure the Build

```bash
meson setup build
```

!!! tip "Build Options"
    You can customize the build with options:
    ```bash
    meson setup build -Dbuildtype=release
    meson setup build -Dbuildtype=debug
    ```

### 3. Compile

```bash
meson compile -C build
```

### 4. Run Tests

```bash
meson test -C build
```

### 5. Verify Installation

```bash
./build/main_improved --help
```

## Environment Setup

### ROOT Configuration

Ensure ROOT is properly configured:

```bash
source /path/to/root/bin/thisroot.sh
```

Or add to your shell profile:

```bash
# ~/.bashrc or ~/.zshrc
export ROOTSYS=/path/to/root
export PATH=$ROOTSYS/bin:$PATH
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
```

### JLab Farm Setup

On the JLab computing farm:

```bash
module load root/6.28.04
module load gcc/11.2.0
```

## Troubleshooting

!!! warning "Common Issues"

    **Missing ROOT**: Ensure `root-config` is in your PATH
    ```bash
    which root-config
    ```

    **C++20 not supported**: Update your compiler
    ```bash
    g++ --version  # Should be 10+
    ```

    **Meson not found**: Install via pip
    ```bash
    pip install meson ninja
    ```

## Next Steps

- [Quick Start Guide](quickstart.md) - Get running in 5 minutes
- [Your First Analysis](first-analysis.md) - Write your first analysis
