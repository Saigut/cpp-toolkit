from conan import ConanFile
from conans import CMake, tools

# conan command: conan create ./libp2p.conanfile.py local/default

class CppLibp2pConan(ConanFile):
    name = "cpp-libp2p"
    version = "0.1.10"

    # Optional metadata
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of CppLibp2p here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        pass

    def source(self):
        git = tools.Git()
        git.clone("https://github.com/libp2p/cpp-libp2p", "v0.1.10", shallow=True)

    def build(self):
        cmake = CMake(self)
        cmake.definitions["TESTING"] = False
        cmake.definitions["EXAMPLES"] = False
        cmake.definitions["CLANG_FORMAT"] = False
        cmake.definitions["EXPOSE_MOCKS"] = False
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["libp2p"]
