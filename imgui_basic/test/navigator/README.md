# Navigator Tests

This folder contains automated tests for the Navigator component using ImGui Test Engine.

## Files

- `navigator_test.cpp` - Main test application with ImGui Test Engine integration
- `navigator_test_helper.h/cpp` - Test utility functions for Navigator testing
- `build_navigator_test.bat` - Build script for the test application

## Building and Running

### Prerequisites
- Visual Studio with C++17 support
- Run from "x64 Native Tools Command Prompt for VS"

### Build
```bash
build_navigator_test.bat
```

### Run Tests
```bash
build\navigator_test.exe
```

## Test Coverage

The test suite includes:

1. **Basic Functionality Tests**
   - Window existence verification
   - Tree structure validation
   - Node expansion/collapse behavior

2. **Navigation Tests**
   - Path-based tree navigation
   - Leaf node accessibility
   - Tree state persistence

3. **Stability Tests**
   - Memory leak detection
   - Rapid interaction stress tests
   - State consistency validation

## Test Structure

Tests are organized into categories:
- `navigator/window_exists` - Basic window presence
- `navigator/tree_structure` - Tree hierarchy validation
- `navigator/all_leaf_nodes` - Comprehensive leaf testing
- `navigator/memory_stability` - Memory management tests
- `navigator/rapid_clicking` - Stress testing
- `navigator/tree_state_persistence` - State management

## Adding New Tests

To add new tests:

1. Add test functions to `navigator_test_helper.cpp`
2. Register tests in `RegisterAllNavigatorTests()`
3. Use `IM_CHECK()` macros for assertions
4. Follow the pattern: `t->TestFunc = [](ImGuiTestContext* ctx) { ... }`
