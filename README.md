# hh
Handmade Hero Tutorial - Electronic Market Simulation Platform (EMSP)

## Project Overview
This project is a high-performance market data processing application built with Dear ImGui, featuring:
- Real-time market data visualization
- Multi-producer single-consumer queue processing
- Comprehensive testing infrastructure
- Plugin-based architecture for market data sources

## Build Prerequisites

### Required Software
1. **Visual Studio 2022** (or compatible MSVC toolchain)
   - Required for C++17 compilation on Windows

2. **CMake 3.16 or higher**
   - Used for cross-platform build configuration

3. **PowerShell Core (pwsh.exe)**
   - **Installation**: `winget install Microsoft.PowerShell`
   - **Why needed**: Google Test's internal test discovery and CTest operations use PowerShell Core for improved cross-platform compatibility and performance
   - **Note**: This is different from Windows PowerShell (powershell.exe) that comes with Windows
   - **Without this**: You'll see warnings like `'pwsh.exe' is not recognized as an internal or external command` during builds

### Optional (for GUI builds)
4. **vcpkg** (if building with ImGui)
   - For managing dependencies like GLFW and GLAD
   - Only needed when `WITH_IMGUI=ON`

## Quick Start

### Building Tests Only (Recommended for CI/Development)
```bash
cd imgui_opengl_glad
# Use the dedicated test build script (includes prerequisite checks)
.\build_tests_only.bat

# OR manual commands:
cmake -B build_tests -S . -DBUILD_TESTS=ON -DWITH_IMGUI=OFF
cmake --build build_tests --config Debug --target unit_tests
cd build_tests && ctest -C Debug --verbose
```

### Building Full GUI Application
```bash
cd imgui_opengl_glad
# Use the GUI build script (includes vcpkg setup and dependency management)
.\build_gui.bat

# OR manual commands (after setting up vcpkg):
cmake -B build_gui -S . -DBUILD_TESTS=OFF -DWITH_IMGUI=ON -DCMAKE_TOOLCHAIN_FILE=c:/dvlp/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build_gui --config Debug
```

## Build Scripts
- **`build_tests_only.bat`** - Fast tests-only build (uses `build_tests/` directory, no GUI dependencies)
- **`build_gui.bat`** - GUI application build (uses `build_gui/` directory, includes vcpkg setup)
- **`build.bat`** - Interactive selector (currently empty - user can customize)

**Note**: The build scripts use separate build directories to avoid configuration conflicts:
- Tests use `build_tests/` and build without external dependencies
- GUI builds use `build_gui/` and include vcpkg dependencies (GLFW, GLAD)

## Testing
The project includes a comprehensive test suite with 14+ test cases covering:
- Core data structures (MPSCQueue, EmspConfig)
- Market data processing logic
- Edge cases and performance characteristics

Run tests with:
```bash
# After running build_tests_only.bat
cd build_tests
ctest -C Debug --output-on-failure --verbose

# OR run the executable directly
.\Debug\unit_tests.exe
```

## Project Structure
- `imgui_opengl_glad/` - Main application code
- `imgui_opengl_glad/tests/` - Comprehensive test suite using Google Test
- `imgui_opengl_glad/plugins/` - Plugin architecture for market data sources
