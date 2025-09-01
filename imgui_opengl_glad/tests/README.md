# Testing Infrastructure

This project uses Google Test (GTest) framework for unit testing and CMake's CTest for test execution.

## Directory Structure

```
imgui_opengl_glad/
├── tests/
│   ├── unittests/
│   │   └── test_data_updater.cpp    # GTest unit tests for data_updater module
│   ├── CMakeLists.txt               # Test configuration
│   └── README.md                    # Testing documentation
├── data_updater.cpp                 # Module under test
├── data_updater.h                   # Module header
└── CMakeLists.txt                   # Main build configuration
```

## Building and Running Tests

### Prerequisites

1. **CMake 3.16+** - For build system  
2. **Visual Studio 2022** (Windows) or **GCC/Clang** (Linux)
3. **Internet connection** - GTest is automatically downloaded via FetchContent

### Local Development

#### Windows (PowerShell)
```powershell
# Quick build and test
.\build_and_test.bat

# Manual steps
cmake -B build -S . -DBUILD_TESTS=ON
cmake --build build --config Debug
cd build; ctest --build-config Debug --verbose
```

#### Linux
```bash
# Install dependencies
sudo apt-get install -y libglfw3-dev libgl1-mesa-dev

# Build and test
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build
cd build && ctest --verbose
```

### Available Tests

The test suite includes:
- **ProcessesQueuedUpdates** - Verifies queue processing logic
- **IgnoresInvalidRowIds** - Tests bounds checking
- **ProcessesSnapshotsWhenTimeReached** - Tests timing-based processing
- **SkipsProcessingWhenTimeNotReached** - Tests timing conditions
- **HandlesConcurrentWrites** - Tests thread-safety aspects
- **CompleteWorkflow** - End-to-end workflow testing
- **HandlesEmptyQueue** - Edge case testing
- **PerformanceCharacteristics** - Basic performance validation

## Continuous Integration

The project uses GitHub Actions for CI/CD with the following workflow:

### On every push to `main` or `develop`:
1. **Build on Windows** - Uses Visual Studio 2022 toolchain
2. **Build on Linux** - Uses system packages and GCC  
3. **Run all tests** - Both platforms execute the full test suite
4. **Code quality checks** - clang-format and static analysis

### On pull requests:
- All CI checks must pass before merging
- Test results are uploaded as artifacts for review

## Test Configuration

Tests are configured via CMake with the following options:
- `BUILD_TESTS=ON` - Enable test building (default: ON)
- Tests have a 30-second timeout
- CTest integration for standardized test execution
- Cross-platform compatibility (Windows/Linux)

## Adding New Tests

1. Create test files in `tests/unittests/`
2. Follow the GTest framework patterns
3. Update `tests/CMakeLists.txt` if needed
4. Ensure tests pass locally before committing
5. CI will automatically run new tests on all platforms

## Best Practices

- Write tests before implementing features (TDD)
- Keep tests fast and focused
- Use descriptive test names
- Test both success and failure cases
- Mock external dependencies when needed
- Maintain high test coverage for critical code paths
