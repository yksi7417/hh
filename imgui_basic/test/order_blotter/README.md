# Order Blotter Tests

This directory contains automated tests for the OrderBlotter component using ImGui Test Engine.

## Test Coverage

### 1. Row Count Correctness Tests
- **Test**: `test_row_count_accuracy`
- **Purpose**: Verify that the displayed row count matches the actual data size
- **Test Cases**:
  - Small dataset (10 rows)
  - Medium dataset (100 rows) 
  - Large dataset (300 rows)
  - Empty dataset (0 rows)
- **Verification**: Compare `Total Orders` display with actual vector size

### 2. Sorting Logic Tests
- **Test**: `test_sorting_functionality`
- **Purpose**: Verify that sorting works correctly for all columns
- **Test Cases**:
  - Sort by Order ID (ascending/descending)
  - Sort by Customer name (alphabetical)
  - Sort by Product name (alphabetical)
  - Sort by Quantity (numerical)
  - Sort by Price (numerical)
  - Sort by Status (alphabetical)
- **Verification**: Check first and last visible rows after sorting

### 3. Variable Row Size Tests
- **Test**: `test_variable_row_sizes`
- **Purpose**: Ensure the table renders correctly with different dataset sizes
- **Test Cases**:
  - 5 rows
  - 50 rows
  - 150 rows
  - 299 rows (edge case)
- **Verification**: Table structure remains intact, scrolling works properly

### 4. Column Display Tests
- **Test**: `test_column_display`
- **Purpose**: Verify all columns are visible and display correct data
- **Test Cases**:
  - Check all 6 columns are present (Order ID, Customer, Product, Quantity, Price, Status)
  - Verify column headers are correct
  - Check data formatting (price shows $, quantity is integer)
  - Verify status color coding works
- **Verification**: Column headers match expected text, data formats correctly

### 5. Multiple Selection Tests
- **Test**: `test_multiple_selection`
- **Purpose**: Verify multi-select functionality works correctly
- **Test Cases**:
  - Single row selection
  - Ctrl+Click multiple selection
  - Shift+Click range selection
  - Clear selection functionality
  - Selection persistence after sorting
- **Verification**: Selected Order IDs match expected values

## Test Files

- `order_blotter_test.cpp` - Main test implementation
- `order_blotter_test_helper.cpp` - Test helper functions
- `order_blotter_test_helper.h` - Test helper declarations
- `build_order_blotter_test.bat` - Build script
- `run_tests.bat` - Test execution script

## Running Tests

### From Visual Studio Command Prompt:
```cmd
cd test\order_blotter
run_tests.bat
```

### Manual Build and Run:
```cmd
cd test\order_blotter
build_order_blotter_test.bat
build\order_blotter_test.exe
```

## Test Implementation Strategy

### Setup Phase
1. Initialize minimal ImGui context
2. Create OrderBlotter instance
3. Load test data of specified size

### Test Execution
1. Render one frame to populate UI
2. Use ImGui Test Engine to interact with UI elements
3. Capture and verify results
4. Clean up test data

### Assertions
- Use ImGui Test Engine's built-in assertion system
- Custom assertions for OrderBlotter-specific functionality
- Screenshot capture for visual regression testing

## Expected Test Results

All tests should pass with:
- ✅ Correct row counts displayed
- ✅ Proper sorting behavior
- ✅ Stable rendering across different row sizes
- ✅ All columns visible and formatted correctly
- ✅ Multi-selection working as expected

## Continuous Integration

These tests can be integrated into CI/CD pipeline:
1. Build the test executable
2. Run tests in headless mode
3. Parse test results for pass/fail status
4. Generate test reports

## Debugging Failed Tests

If tests fail:
1. Check the console output for assertion messages
2. Review screenshot captures (if enabled)
3. Run individual tests in isolation
4. Verify test data generation is consistent
5. Check ImGui Test Engine compatibility
