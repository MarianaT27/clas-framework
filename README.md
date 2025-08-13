# CLAS12 Analysis

A minimalistic C++ repository to kickstart your CLAS12 data analysis using the HIPO library.

## Features

- Ready-to-use C++ analysis framework for CLAS12 data.
- Integrates the HIPO library for efficient I/O.
- Modular code structure for easy extension.
- Example code for reading and drawing data.

## Dependencies

- **Meson** (build system): [mesonbuild.com](https://mesonbuild.com/)
- **Ninja** (build tool): [ninja-build.org](https://ninja-build.org/)
- **C++17** or newer compiler

You can install Meson and Ninja via your package manager (e.g., `brew`, `apt`, `dnf`). If your package manager provides outdated versions, use pip:

```sh
python -m pip install meson ninja
```

## Building the Project

1. Configure the build directory:
    ```sh
    meson setup build
    ```
2. Compile the project:
    ```sh
    cd build
    meson compile
    ```

## Usage

- Place your configuration files in the `config/` directory.
- Input data files should be stored in the `data/` directory.
- Source code is organized under `src/` and headers under `include/`.

To run the main analysis executable after building:
```sh
./build/starter
```

## Project Structure

```
├── src/           # Source files
├── include/       # Header files
├── config/        # Configuration files (e.g., TOML)
├── data/          # Example data files
├── hipo4/         # HIPO library sources
├── subprojects/   # External dependencies
├── meson.build    # Build configuration
```

## Contributing

Pull requests and issues are welcome! Please follow standard C++ coding conventions and document your code.

## License

This project is licensed under the MIT License. See `LICENSE` for details.


