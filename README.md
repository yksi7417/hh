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

### Optional (for enhanced linting)
5. **clang-tidy** (part of LLVM/Clang tools)
   - **Installation**: Download from [LLVM Releases](https://releases.llvm.org/) or via package manager
   - **Why useful**: Industry-standard static analysis for additional code quality checks
   - **Note**: The linter works without it, but provides more comprehensive analysis with it

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

## Code Quality and Linting

The project enforces design principles and code quality through automated linting:

### **Run Code Quality Checks**
```bash
cd imgui_opengl_glad

# Run all quality checks (design principles + static analysis)
.\lint.bat

# Check specific files only
.\lint.bat core/data_updater.cpp ui/MarketDataTable.cpp
```

### **Design Principles Enforced**
- ❌ Raw `new`/`delete` without documentation justification
- ❌ `malloc`/`free` usage in C++ code (use RAII instead)
- ❌ Performance-critical code without documentation
- ✅ Document with nearby comments for exceptions

### **Valid Documentation Examples**
```cpp
// Performance critical: tight loop for market data processing
while (ctx.q.pop(id)) { ... }

/// @brief Legacy API compatibility requires raw pointer management
MyType* ptr = new MyType();

/** NOTE: Manual memory management required for plugin interface */
delete plugin_instance;
```

### **Install Pre-commit Hook (Recommended)**
```bash
# Copy the pre-commit hook
copy pre-commit-hook-example .git\hooks\pre-commit

# On Linux/Mac, make it executable:
# chmod +x .git/hooks/pre-commit
```

The pre-commit hook will automatically run code quality checks before each commit, preventing violations from entering the repository.

### **Troubleshooting Linting**
```bash
# If Python is not found
winget install Python.Python.3.12

# If you get "execution policy" errors on Windows
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser

# Skip linting for urgent commits (not recommended)
git commit --no-verify -m "urgent fix"
```

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
  - `core/` - Business logic, data processing (no UI dependencies)
  - `ui/` - User interface components (ImGui-dependent)
- `imgui_opengl_glad/tests/` - Comprehensive test suite using Google Test
- `imgui_opengl_glad/plugins/` - Plugin architecture for market data sources
