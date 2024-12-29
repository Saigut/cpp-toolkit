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
# You can specify your compiler and generator in blow command
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel --config Release
```
