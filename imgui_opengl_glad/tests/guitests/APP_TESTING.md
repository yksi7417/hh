# Testing Your Application GUI

## Overview

You now have **two types of tests** in this project:

1. **Simple GUI Tests** (`simple_gui_test.cpp`) - Basic ImGui widget tests
2. **Application GUI Tests** (`app_gui_test.cpp`) - Tests for your actual Market Data Table

## How It Works

### Architecture

```
┌─────────────────────────────────────┐
│   simple_gui_test.exe               │
│                                     │
│  ┌──────────────────────────────┐  │
│  │  Test Engine                 │  │
│  │  - Manages test execution    │  │
│  │  - Simulates user input      │  │
│  └──────────────────────────────┘  │
│                                     │
│  ┌──────────────────────────────┐  │
│  │  Your GUI Components         │  │
│  │  - ImGuiComponents           │  │
│  │  - MarketDataTable           │  │
│  │  - Same code as main.cpp     │  │
│  └──────────────────────────────┘  │
│                                     │
│  ┌──────────────────────────────┐  │
│  │  Mock Data                   │  │
│  │  - HostContext               │  │
│  │  - Test market data          │  │
│  └──────────────────────────────┘  │
└─────────────────────────────────────┘
```

### Linking Strategy

The tests use **the exact same GUI code** as your `main.cpp`:

1. **Shared Source Files** (in `CMakeLists.txt`):
   ```cmake
   ${APP_DIR}/ui/IMGuiComponents.cpp
   ${APP_DIR}/ui/MarketDataTable.cpp
   ${APP_DIR}/core/data_updater.cpp
   ```

2. **Mock Data Instead of Plugin**:
   - Tests create mock `HostContext` and `HostMDSlot`
   - No need to load `md_plugin.dll`
   - You control the data in tests

3. **Test Execution**:
   - `GuiFunc` = Renders your GUI (calls `ImGuiComponents::Update()`)
   - `TestFunc` = Verifies the GUI is correct

## Current Test Status

### ✅ All Tests Passing!
- `simple_gui/basic_window` - Basic window creation
- `simple_gui/button_click` - Button interaction  
- `simple_gui/checkbox` - Checkbox toggle
- `app_gui/market_data_table_window` - Market data window exists
- `app_gui/market_data_table_has_rows` - Table displays mock data
- `app_gui/data_updates` - Data updates work correctly

**Total: 6/6 tests passing** ✅

## How to Fix/Improve Application Tests

### ✅ Solution: Test Components Directly

We successfully test `MarketDataTable` directly instead of through `ImGuiComponents`:

**Why this works:**
- Test Engine provides its own ImGui context
- No need for GLFW window initialization
- Tests the actual production code
- Fast and reliable

**Implementation:**
```cpp
// In app_gui_test.cpp
struct AppGuiTestVars {
    std::unique_ptr<MarketDataTable> market_data_table;
    // ... mock data ...
};

t->GuiFunc = [](ImGuiTestContext* ctx) {
    vars.market_data_table->UpdateFromContext(vars.ctx, vars.slot, should_refresh);
    vars.market_data_table->Render(vars.ctx, vars.slot);
};
```

### Key Lessons Learned

1. **Window Name Matters**: The window is called `"MarketData"` (not "Market Data Table")
2. **Component-Level Testing**: Test individual components rather than the full app wrapper
3. **Mock Data Works**: No need for `md_plugin.dll` - use test data
4. **Test Engine Context**: Reuse the Test Engine's ImGui context instead of creating your own

## Running the Tests

### See All Tests (Manual Mode)
```bash
.\build\Debug\simple_gui_test.exe --manual
```

This opens the GUI where you can:
- See all 6 tests listed
- Run them individually or all at once
- See which ones pass/fail
- Debug failures visually

### CI Mode (Headless)
```bash
.\build\Debug\simple_gui_test.exe --headless
```

Output:
```
Total Tests: 6
Passed: 6
Failed: 0
```

All tests pass! ✅

## Next Steps

Now that all tests pass, you can:

1. **Add More Specific Assertions**: Test table contents, sorting, filtering
2. **Test User Interactions**: Click headers, filter data, group by columns
3. **Performance Tests**: Verify rendering speed with large datasets
4. **Edge Cases**: Empty tables, malformed data, boundary conditions
5. **CI Integration**: Add `--headless` tests to your CI/CD pipeline

## Benefits of This Approach

✅ **Code Reuse**: Same GUI code in both app and tests
✅ **No Duplication**: Don't maintain separate test UI code
✅ **Real Testing**: Tests run actual production code
✅ **Fast Iteration**: No need to run full app with plugin
✅ **Controlled Data**: You control test scenarios
✅ **CI Ready**: Can run headless once stabilized

## Example: Testing Specific Functionality

Once basic tests work, you can add detailed tests:

```cpp
// Test that price updates are displayed correctly
t->TestFunc = [](ImGuiTestContext* ctx) {
    AppGuiTestVars& vars = ctx->GetVars<AppGuiTestVars>();
    
    // Set a specific price
    vars.px_n[0] = 12345;
    vars.ctx.seq[0].store(1, std::memory_order_relaxed);
    
    ctx->SetRef("Market Data Table");
    
    // TODO: Add check for specific table cell content
    // (requires more advanced Test Engine features)
};
```

## Files

- `app_gui_test.cpp` - Application GUI tests
- `simple_gui_test.cpp` - Basic ImGui tests
- `simple_gui_test_main.cpp` - Test runner
- `CMakeLists.txt` - Links application sources

## Questions?

The tests are now **linked** to your application code. The key decisions are:

1. How much to refactor for testability?
2. What level to test at (integration vs component)?
3. What assertions to add?

Run `--manual` mode to see the current state and decide your next steps!
