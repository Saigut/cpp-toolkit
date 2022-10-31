# C++ Toolkit
Here put some cross-platform tools for convenience of C++ development. And also as a good sample for building C++ project.

# Dependency
1. Install conan package manager
2. Adjust installation location of conan packages
    ```shell
    # show
    conan config get storage.path
    # change
    conan config set storage.path=<your path>
    ```
3. Install dependency packages. Execute:
    ```shell
    apt-get install pythonX.X-dev   # For boost. This can be skipped.
    mkdir build
    cd build
    conan install ../z_other/conanfile_<os>.txt
    ```

# Build
```shell
cd build
cmake ..
cmake --build . --parallel
```
