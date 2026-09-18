from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout, CMake

class TinCan(ConanFile):
    name = "tincan"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "CMakeLists.txt", "src/*"

    def validate(self):
        # C++17 is the floor for every moth project.
        check_min_cppstd(self, 17)

    def requirements(self):
        self.requires("moth_anim/0.1.0")
        self.requires("moth_bridge/0.1.0")
        self.requires("moth_ecs/0.1.0")
        self.requires("moth_tilemap/0.1.0")
        self.requires("moth_profile/0.1.0")

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
