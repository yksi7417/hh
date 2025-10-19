# How to Find and Run Unit Tests

## Quick Answer

### Unit Test Locations:
```
📁 Source Files:
   c:\dvlp\hh\imgui_opengl_glad\tests\unittests\
   ├── simple_test.cpp           # Basic GTest example
   └── test_data_updater.cpp     # Data updater module tests

📦 Built Executables:
   c:\dvlp\hh\imgui_opengl_glad\build_tests\tests\Debug\unit_tests.exe
   c:\dvlp\hh\imgui_opengl_glad\build\tests\Debug\unit_tests.exe
```

## How to Build and Run Unit Tests

### Option 1: Quick Build Script (Recommended)
```powershell
cd C:\dvlp\hh
.\build_tests_only.bat
```

This will:
- ✓ Configure CMake for tests only (no GUI dependencies)
- ✓ Build the unit_tests executable
- ✓ Run all tests via CTest
- ✓ Display test results

### Option 2: Manual CMake Build
```powershell
cd C:\dvlp\hh\imgui_opengl_glad

# Configure
cmake -B build_tests -S . -DBUILD_TESTS=ON -DWITH_IMGUI=OFF

# Build
cmake --build build_tests --config Debug --target unit_tests

# Run tests
cd build_tests
ctest --build-config Debug --verbose
```

### Option 3: Run Executable Directly
```powershell
# If already built, just run:
C:\dvlp\hh\imgui_opengl_glad\build_tests\tests\Debug\unit_tests.exe
```

## Test Types in the Project

### 1. Unit Tests (GTest)
**Location**: `imgui_opengl_glad/tests/unittests/`  
**Framework**: Google Test  
**Executable**: `unit_tests.exe`  
**Purpose**: Test individual components in isolation

**Test Files**:
- `simple_test.cpp` - Basic GTest example
- `test_data_updater.cpp` - Tests for data_updater module

**Available Tests**:
- ProcessesQueuedUpdates
- IgnoresInvalidRowIds
- ProcessesSnapshotsWhenTimeReached
- SkipsProcessingWhenTimeNotReached
- HandlesConcurrentWrites
- CompleteWorkflow
- HandlesEmptyQueue
- PerformanceCharacteristics

### 2. GUI Tests (ImGui Test Engine)
**Location**: `imgui_opengl_glad/tests/guitests/`  
**Framework**: ImGui Test Engine  
**Executable**: `simple_gui_test.exe`  
**Purpose**: Test GUI components interactively

**Test Files**:
- `simple_gui_test.cpp` - Basic GUI tests (window, button, checkbox, Navigator)
- `app_gui_test.cpp` - MarketDataTable tests

**Available Tests** (11 total):
- simple_gui/basic_window
- simple_gui/button_click
- simple_gui/checkbox
- simple_gui/navigator_window
- simple_gui/navigator_categories
- simple_gui/navigator_statistics
- simple_gui/navigator_filters
- simple_gui/navigator_empty
- app_gui/market_data_table_window
- app_gui/market_data_table_has_rows
- app_gui/data_updates

## Test Configuration Files

### CMakeLists.txt
**Location**: `imgui_opengl_glad/tests/CMakeLists.txt`

Key settings:
- Uses Google Test via FetchContent (auto-downloads)
- Builds `unit_tests` executable
- Integrates with CTest
- 30-second timeout per test
- C++17 standard

## Common Commands

### Check if tests exist:
```powershell
Get-ChildItem -Path C:\dvlp\hh -Recurse -Filter "unit_tests.exe"
Get-ChildItem -Path C:\dvlp\hh -Recurse -Filter "simple_gui_test.exe"
```

### Run specific test:
```powershell
# Unit tests - run all
C:\dvlp\hh\imgui_opengl_glad\build_tests\tests\Debug\unit_tests.exe

# Unit tests - run with GTest filter
C:\dvlp\hh\imgui_opengl_glad\build_tests\tests\Debug\unit_tests.exe --gtest_filter=*ProcessesQueuedUpdates*

# GUI tests - headless mode
C:\dvlp\hh\imgui_opengl_glad\tests\guitests\build\Debug\simple_gui_test.exe --headless

# GUI tests - interactive mode
C:\dvlp\hh\imgui_opengl_glad\tests\guitests\build\Debug\simple_gui_test.exe --manual
```

### View test list:
```powershell
# List all unit tests
C:\dvlp\hh\imgui_opengl_glad\build_tests\tests\Debug\unit_tests.exe --gtest_list_tests

# Use CTest
cd C:\dvlp\hh\imgui_opengl_glad\build_tests
ctest --build-config Debug --show-only
```

## Directory Structure

```
C:\dvlp\hh\
├── build_tests_only.bat                    # Quick test build script
├── imgui_opengl_glad/
│   ├── tests/
│   │   ├── CMakeLists.txt                  # Test configuration
│   │   ├── README.md                       # Test documentation
│   │   ├── unittests/                      # ← UNIT TEST SOURCE
│   │   │   ├── simple_test.cpp
│   │   │   └── test_data_updater.cpp
│   │   └── guitests/                       # ← GUI TEST SOURCE
│   │       ├── simple_gui_test.cpp
│   │       ├── app_gui_test.cpp
│   │       └── simple_gui_test_main.cpp
│   ├── build_tests/                        # Build output (tests only)
│   │   └── tests/
│   │       └── Debug/
│   │           └── unit_tests.exe          # ← UNIT TEST EXECUTABLE
│   └── build/                              # Full build output
│       └── tests/
│           └── Debug/
│               └── unit_tests.exe          # ← UNIT TEST EXECUTABLE (alt)
```

## Troubleshooting

### "unit_tests.exe not found"
```powershell
# Build tests first:
cd C:\dvlp\hh
.\build_tests_only.bat
```

### "PowerShell not found"
```powershell
# Install PowerShell Core:
winget install Microsoft.PowerShell
```

### Tests fail to build
```powershell
# Check CMake version (need 3.16+):
cmake --version

# Clean and rebuild:
cd C:\dvlp\hh\imgui_opengl_glad
Remove-Item -Recurse -Force build_tests -ErrorAction SilentlyContinue
.\build_tests_only.bat
```

### Need to see test output
```powershell
# Use CTest verbose mode:
cd C:\dvlp\hh\imgui_opengl_glad\build_tests
ctest --build-config Debug --verbose --output-on-failure
```

## Quick Reference Card

| Task | Command |
|------|---------|
| **Build all tests** | `.\build_tests_only.bat` |
| **Run unit tests** | `.\imgui_opengl_glad\build_tests\tests\Debug\unit_tests.exe` |
| **Run GUI tests** | `.\imgui_opengl_glad\tests\guitests\build\Debug\simple_gui_test.exe --headless` |
| **List unit tests** | `unit_tests.exe --gtest_list_tests` |
| **Run specific test** | `unit_tests.exe --gtest_filter=TestName` |
| **CTest verbose** | `ctest --build-config Debug --verbose` |
| **Find test exes** | `Get-ChildItem -Recurse -Filter "*test*.exe"` |

## Related Documentation

- **Test README**: `imgui_opengl_glad/tests/README.md`
- **GUI Tests**: `imgui_opengl_glad/tests/guitests/QUICK_REFERENCE.md`
- **Navigator Tests**: `doc/NAVIGATOR_TESTS.md`
- **Navigator Filters**: `doc/NAVIGATOR_QUICK_FILTERS.md`
