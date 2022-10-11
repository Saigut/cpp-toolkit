# C++ Toolkit
Here put some cross-platform tools for convenience of C++ development. And also as a good sample for building C++ project.

# Dependency
1. Install conan package manager
2. Execute:
```shell
apt-get install pythonX.X-dev   # For boost
mkdir build
cd build
conan install ../z_other
```

# Build
```shell
cd build
cmake ..
cmake --build . --parallel
```