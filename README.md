# C++ Toolkit
Here put some cross-platform tools for convenience of C++ development. And also as a good sample for building C++ project.

## Dependency
1. Install conan 2.x package manager
2. Adjust installation location of conan packages
   
   ```
   conan cache clean
   ```
   
   Windows: C:\Users\<your_username>\.conan2\global.conf
   Linux/MacOS: ~/.conan2/global.conf
   
   ```shell
   core.cache:storage_path = xxxx
   core.download:download_cache = xxxx
   ```

## Build
```shell
mkdir build
cd build
cmake ..
cmake --build . --parallel
```
**Caution:** `conan install` maybe failed when `cmake ..` running, causing by compiler settings. Then you can refer to below settings for conan config file (eg: `~/.conan/profiles/default`, `[settings]` section, with gcc 9 and using c++11 ABI)
   ```
   compiler=gcc
   compiler.version=9
   compiler.libcxx=libstdc++11
   ```
