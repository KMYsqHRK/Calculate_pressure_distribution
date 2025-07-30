# Pressure Distribution Solver

A C++ program that calculates pressure distribution using the Poisson equation.

## Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)
- Git

## Building the Project

### 1. Clone the Repository

```bash
git clone <repository-url>
cd Calculate_pressure_distribution
```

### 2. Initialize Eigen Submodule

This project uses Eigen as a git submodule. Initialize and update it:

```bash
git submodule update --init --recursive
```

### 3. Build with CMake

Create a build directory and compile:

```bash
mkdir build
cd build
cmake ..
make
```

The executable `PressureDistSolver` will be created in the build directory.

### 4. Run the Program

```bash
./PressureDistSolver
```

## Managing Eigen Library

### About Eigen

This project uses [Eigen](https://eigen.tuxfamily.org/), a C++ template library for linear algebra. Eigen is included as a git submodule located in `third_party/eigen/`.

### Submodule Commands

- **Initialize submodules** (after cloning):
  ```bash
  git submodule update --init --recursive
  ```

- **Update Eigen to latest version**:
  ```bash
  git submodule update --remote third_party/eigen
  ```

- **Check submodule status**:
  ```bash
  git submodule status
  ```

- **If submodule issues occur**, reset and reinitialize:
  ```bash
  git submodule deinit third_party/eigen
  git submodule update --init --recursive
  ```

### Eigen Version

The project is configured to work with Eigen 3.x. The specific version is managed through the git submodule commit hash.

## Project Structure

```
├── src/                    # Source code
│   ├── main.cpp           # Main program entry point
│   ├── pressuredistsolver.cpp  # Pressure distribution solver implementation
│   ├── pressuredistsolver.hpp  # Solver header file
│   ├── csv_reader.cpp     # CSV file reader implementation
│   └── csv_reader.hpp     # CSV reader header file
├── CMakeLists.txt         # CMake configuration
├── .gitmodules           # Git submodule configuration
└── third_party/eigen/    # Eigen library (submodule)
```

## Troubleshooting

### Build Errors

- If you get "Eigen library not found" error, make sure to run:
  ```bash
  git submodule update --init --recursive
  ```

- If compilation fails with missing algorithm header, ensure you have a C++17 compatible compiler.

### Submodule Issues

- If the `third_party/eigen` directory is empty, reinitialize submodules:
  ```bash
  git submodule update --init --recursive
  ```

- If submodule updates fail, check your git configuration and network connection.