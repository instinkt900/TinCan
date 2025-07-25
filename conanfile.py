from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMake

class TinCan(ConanFile):
    name = "tincan"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "MSBuildToolchain", "MSBuildDeps"
    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("canyon/0.2.0")
        self.requires("entt/3.15.0")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.27.0]")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

