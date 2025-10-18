# Navigator Tests Documentation

## Overview
This document describes the ImGui Test Engine tests added for the Navigator component. The tests are integrated into the existing `simple_gui_test.cpp` file.

## Test File
- **Location**: `tests/guitests/simple_gui_test.cpp`
- **Test Group**: `simple_gui`
- **Tests**: `navigator_window`, `navigator_categories`, `navigator_statistics`, `navigator_filters`, `navigator_empty`

## Tests Added

### 1. `simple_gui/navigator_window`
**Purpose**: Basic smoke test to verify Navigator window shows up

**What it tests**:
- Navigator window is created and rendered
- Window contains drawable content (not empty)

**Test Data**: Empty/default data (100 rows initialized but not populated)

---

### 2. `simple_gui/navigator_categories`
**Purpose**: Verify Data Categories tree node is present and functional

**What it tests**:
- Data Categories tree node exists and can be opened
- "By Side" sub-tree can be opened
- Navigator renders with mixed buy/sell order data

**Test Data**: 50 rows with varied buy/sell sides and prices

---

### 3. `simple_gui/navigator_statistics`
**Purpose**: Verify Statistics tree node is present

**What it tests**:
- Statistics tree node exists and can be opened
- Statistics are calculated and displayed
- Window remains valid after opening statistics

**Test Data**: 20 rows with specific values:
- 10 buy orders, 10 sell orders
- Fixed price of 150.00
- Fixed quantity of 100
- 5 rows marked as dirty

---

### 4. `simple_gui/navigator_filters`
**Purpose**: Verify Quick Filters tree node is present

**What it tests**:
- Quick Filters tree node exists and can be opened
- Window remains valid after opening filters

**Test Data**: 10 rows with default data

---

### 5. `simple_gui/navigator_empty`
**Purpose**: Edge case test for empty data handling

**What it tests**:
- Navigator renders correctly with 0 rows
- Tree nodes can still be opened with no data
- No crashes or errors with empty data

**Test Data**: 0 rows (empty dataset)

---

## Running the Tests

### Manual/Interactive Mode
```bash
cd imgui_opengl_glad/tests/guitests/build
./simple_gui_test.exe --manual
```
Then use the Test Engine UI to run Navigator tests.

### Headless/Automated Mode
```bash
cd imgui_opengl_glad/tests/guitests/build
./simple_gui_test.exe --headless
```
All tests run automatically and exit with status code.

### Run Specific Navigator Tests
In manual mode, you can filter by:
- Category: `simple_gui`
- Individual tests: `simple_gui/navigator_window`, `simple_gui/navigator_categories`, etc.

## Test Structure

The Navigator tests are integrated into `simple_gui_test.cpp`:

1. **NavigatorTestVars** struct holds:
   - Mock `HostContext` and `HostMDSlot`
   - Test data vectors (timestamps, prices, quantities, sides)
   - Navigator instance
   - Initialization state

2. **GuiFunc**: Renders the Navigator each frame
   - Initializes test data
   - Calls `navigator->Render(ctx, slot)`

3. **TestFunc**: Verifies expected behavior
   - Checks window exists
   - Opens tree nodes
   - Validates rendering

## Integration

The Navigator tests are part of the simple GUI test suite:

1. **simple_gui_test.cpp** - Contains all basic GUI tests plus Navigator tests
2. **simple_gui_test_main.cpp** - Registers tests via `RegisterSimpleGuiTests()`
3. **CMakeLists.txt** - Already includes `simple_gui_test.cpp` in build

## Next Steps

You can extend these tests to add:
- Verification of specific text/labels in the Navigator
- Click interactions with tree nodes to verify expand/collapse
- Filter activation tests (when filters are implemented)
- Interaction between Navigator and MarketDataTable
- Performance tests with large datasets
- Tests for specific data aggregations/statistics
- Tests for different price ranges

## Notes

- Tests use the ImGui Test Engine's context system
- Each test is isolated with its own `NavigatorTestVars`
- Tests can run in parallel (Test Engine handles synchronization)
- The Navigator uses the same `HostContext`/`HostMDSlot` structure as MarketDataTable
- All tests are in the `simple_gui` category for consistency with existing basic tests
