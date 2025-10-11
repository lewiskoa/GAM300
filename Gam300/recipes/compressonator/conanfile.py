from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import cmake_layout, CMake, BasicCMakeDeps
from conan.tools.files import git, run, copy
from conan.tools.build import check_min_cppstd
from conan.tools.env import VirtualBuildEnv, VirtualRunEnv
from conan.tools.microsoft import MSVCCompilerFlag, is_msvc
import os

class CompressonatorConan(ConanFile):
    name = "compressonator"
    version = "4.5.52"  # Use the latest release tag; update as needed
    package_type = "library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "freetype": [True, False],
        "enable_gpu": [True, False],  # Enable GPU (OpenCL/DX) plugins
        "build_gui": [True, False]    # Optional: Build GUI (adds Qt deps)
    }
    default_options = {
        "shared": False,             # Static by default (like your assimp)
        "freetype": True,
        "enable_gpu": True,
        "build_gui": False           # Disable GUI for SDK-only
    }

    # Minimum C++17 to match your profile
    def configure(self):
        check_min_cppstd(self, 17)

    def requirements(self):
        # External deps fetched by project script; add if needed as Conan requires
        # self.requires("zlib/1.3.1")  # Example; check fetch_dependencies.py output
        if self.options.enable_gpu:
            # For OpenCL/DX; assume system-provided or add recipes
            pass
        if self.options.build_gui:
            self.requires("qt/6.5.0")  # If GUI enabled

    def build_requirements(self):
        self.tool_requires(self.tested_reference_str)  # Self for CMake

    def layout(self):
        cmake_layout(self, src_folder="src")

    def source(self):
        git(self, "https://github.com/GPUOpen-Tools/compressonator.git", target="src", branch=f"v{self.version}")

    def generate(self):
        # Virtual env for build isolation
        env = VirtualBuildEnv(self)
        env.apply()
        # Add MSVC runtime flag for dynamic (matches your profile)
        if is_msvc(self) and self.settings.compiler.runtime == "dynamic":
            env.append_to_cmake_defs("CMAKE_MSVC_RUNTIME_LIBRARY", "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

        # Deps generator for CMake
        tc = BasicCMakeDeps(self)
        tc.generate()

    def build(self):
        # Create Common folder for external deps (above src)
        common_dir = os.path.join(self.build_folder, "..", "Common")
        os.makedirs(common_dir, exist_ok=True)

        # Run project's dependency fetch script (requires Python)
        python_script = os.path.join(self.source_folder, "build", "fetch_dependencies.py")
        run(self, f"python {python_script} --common_dir {common_dir}", env="env")

        # Set CMake defs for deps and options
        cmake = CMake(self)
        cmake.configure(
            defs=[
                f"-DOPTION_BUILD_FRAMEWORK_SDK=ON",  # Core + Framework
                f"-DOPTION_BUILD_APPS_CMP_CLI=OFF",  # Disable CLI (SDK focus)
                f"-DOPTION_ENABLE_ALL_APPS=OFF",
                f"-DOPTION_BUILD_GPU_DECODE={'ON' if self.options.enable_gpu else 'OFF'}",
                f"-DOPTION_BUILD_GUI={'ON' if self.options.build_gui else 'OFF'}",
                f"-DFREETYPE_ENABLED={'ON' if self.options.freetype else 'OFF'}",
                f"-DCOMMON_LIB_DIR={common_dir}"  # Point to fetched deps
            ],
            build_dir="build"  # Subfolder to avoid pollution
        )
        cmake.build()

    def package(self):
        # Copy headers
        copy(self, "include", os.path.join(self.source_folder, "source"), os.path.join(self.package_folder, "include"))
        copy(self, "compressonator.h", self.source_folder, self.package_folder / "include")  # Main header

        # Copy libs (adjust paths based on build output)
        lib_folder = os.path.join(self.package_folder, "lib")
        bin_folder = os.path.join(self.package_folder, "bin")
        os.makedirs(lib_folder, exist_ok=True)
        os.makedirs(bin_folder, exist_ok=True)

        # Static/shared libs (e.g., CMP_Framework.lib / .dll)
        if self.options.shared:
            copy(self, "*.dll", os.path.join(self.cpp.build.libdirs[0], "CMP_*"), bin_folder, root_package="compressonator")
            copy(self, "*.lib", os.path.join(self.cpp.build.libdirs[0], "CMP_*"), lib_folder, root_package="compressonator")  # Import lib
        else:
            copy(self, "*.lib", os.path.join(self.cpp.build.libdirs[0], "CMP_*"), lib_folder, root_package="compressonator")

        # GPU plugins/DLLs if enabled
        if self.options.enable_gpu:
            plugins_dir = os.path.join(self.package_folder, "plugins", "compute")
            os.makedirs(plugins_dir, exist_ok=True)
            copy(self, "*.dll", self.cpp.build.bindirs[0], plugins_dir, patterns="CMP_GPU_*")  # e.g., CMP_GPU_OCL_MD.dll
            copy(self, "*.hlsl", self.source_folder, plugins_dir)  # Kernel files
            copy(self, "*.cpp", self.source_folder, plugins_dir, patterns="BC*_Encode_Kernel.cpp")  # Kernel sources

    def package_info(self):
        # For MSBuildDeps integration
        self.cpp.includedirs = ["include"]
        self.cpp.libdirs = ["lib"]
        self.cpp.bindirs = ["bin"]
        self.cpp.libs = ["CMP_Framework", "CMP_Core"]  # Main libs; add more as needed (e.g., CMP_ImageIO)
        if self.options.enable_gpu:
            self.cpp.libs.extend(["CMP_GPU_OCL", "CMP_GPU_DXC"])  # GPU DLLs
            self.cpp.resdirs.append("plugins/compute")  # Runtime plugin path

        # Runtime paths for DLLs/plugins
        if self.options.shared:
            self.cpp.frameworkpaths = ["bin", "plugins/compute"]