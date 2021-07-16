# C++ Toolkit
Here put some cross-platform tools for convenience of C++ development. And also as a good sample for building C++ project.

## Dependency
`apt-get install pythonX.X-dev`   # For boost  
`apt install libsqlite3-dev`      # For libp2p

## Issues
* ssl and crypto library installed by grpc(?) can cause issues with asio!

* libp2p-src/multi/converters/converter_utils.cpp:127  
  `gsl::span<const uint8_t, -1>`
  should change to
  `gsl::span<const uint8_t, (long unsigned int)-1>`?
