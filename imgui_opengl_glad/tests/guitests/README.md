# Simple GUI Tests with ImGui Test Engine

This directory contains the **simplest possible examples** of using ImGui Test Engine to automate GUI testing.

## Files

- **`simple_imconfig.h`** - ImGui configuration for test engine integration
- **`simple_gui_test.cpp`** - Three basic test examples demonstrating core concepts
- **`simple_gui_test_main.cpp`** - Minimal test runner application
- **`build_simple_test.bat`** - Build script to compile and link everything

## The Three Simple Tests

### Test 1: `basic_window`
**Purpose**: Most basic test - just creates a window and verifies it exists.

```cpp
t->GuiFunc = [](ImGuiTestContext* ctx) {
    ImGui::Begin("Simple Test Window");
    ImGui::Text("Hello, Test Engine!");
    ImGui::End();
};
t->TestFunc = [](ImGuiTestContext* ctx) {
    ctx->SetRef("Simple Test Window");
    IM_CHECK(ctx->GetWindowByRef("") != nullptr);
};
```

**What it demonstrates**: 
- Creating a GUI window in `GuiFunc`
- Finding and verifying the window in `TestFunc`

### Test 2: `button_click`
**Purpose**: Tests clicking a button and verifying state changes.

```cpp
struct ButtonTestVars { int ClickCount = 0; };
t->SetVarsDataType<ButtonTestVars>();
t->GuiFunc = [](ImGuiTestContext* ctx) {
    ButtonTestVars& vars = ctx->GetVars<ButtonTestVars>();
    if (ImGui::Button("Click Me"))
        vars.ClickCount++;
};
t->TestFunc = [](ImGuiTestContext* ctx) {
    ButtonTestVars& vars = ctx->GetVars<ButtonTestVars>();
    ctx->ItemClick("Click Me");
    IM_CHECK_EQ(vars.ClickCount, 1);
};
```

**What it demonstrates**:
- Sharing data between `GuiFunc` and `TestFunc` using vars
- Simulating button clicks with `ctx->ItemClick()`
- Verifying state changes with `IM_CHECK_EQ()`

### Test 3: `checkbox`
**Purpose**: Tests checking/unchecking a checkbox.

```cpp
struct CheckboxVars { bool IsChecked = false; };
t->GuiFunc = [](ImGuiTestContext* ctx) {
    CheckboxVars& vars = ctx->GetVars<CheckboxVars>();
    ImGui::Checkbox("Test Checkbox", &vars.IsChecked);
};
t->TestFunc = [](ImGuiTestContext* ctx) {
    ctx->ItemCheck("Test Checkbox");
    IM_CHECK_EQ(vars.IsChecked, true);
    ctx->ItemUncheck("Test Checkbox");
    IM_CHECK_EQ(vars.IsChecked, false);
};
```

**What it demonstrates**:
- Using `ctx->ItemCheck()` and `ctx->ItemUncheck()`
- Verifying boolean state changes

## Key Concepts

### 1. Test Structure
Every test has two main parts:
- **`GuiFunc`**: Renders your GUI (called every frame)
- **`TestFunc`**: Performs automated interactions and verifications

### 2. Test Context (`ctx`)
The `ImGuiTestContext*` provides:
- **`SetRef()`**: Set the current window/widget reference
- **`ItemClick()`**: Simulate clicking a button
- **`ItemCheck()/ItemUncheck()`**: Toggle checkboxes
- **`GetVars<T>()`**: Access shared test variables

### 3. Assertions
- **`IM_CHECK(condition)`**: Assert condition is true
- **`IM_CHECK_EQ(a, b)`**: Assert values are equal

### 4. Variables
Use `SetVarsDataType<T>()` to share data between GuiFunc and TestFunc:
```cpp
struct MyVars { int counter = 0; };
t->SetVarsDataType<MyVars>();
// Access with: ctx->GetVars<MyVars>()
```

## Building and Running

### Build
```bash
cd imgui_opengl_glad\tests\guitests
cmake -B build -S .
cmake --build build --config Debug

# Or use the batch file
build_simple_test.bat
```

### Run - Manual Mode (Default)

Run the test application with GUI to manually run tests or observe automated test execution:

```bash
# Windows - all equivalent
run_test.bat
build\Debug\simple_gui_test.exe
build\Debug\simple_gui_test.exe --manual
```

**What You'll See**:
1. A window titled "Simple GUI Test Runner"
2. The **Test Engine** window showing:
   - List of registered tests
   - Run controls (Run All, Run Selected)
   - Test results (Pass/Fail)
3. Your test windows appearing during execution

### Run - Headless Mode (CI/Automated Testing)

Run all tests automatically without showing the UI window, then exit with results:

```bash
# Windows
run_headless.bat

# Or directly
build\Debug\simple_gui_test.exe --headless
```

The headless mode:
- ✅ Queues all tests automatically
- ✅ Runs in fast mode (no throttling)
- ✅ Exits after all tests complete
- ✅ Returns exit code 0 if all pass, non-zero if any fail
- ✅ Perfect for CI/CD pipelines

**Example Output**:
```
Test Mode: Headless (Automated)
...
All tests completed in headless mode. Exiting...

=== Test Results Summary ===
Total Tests: 3
Passed: 3
Failed: 0
===========================
```

### Command-Line Options

```
Usage: simple_gui_test.exe [options]
Options:
  --headless      Run tests headlessly and exit (for CI)
  --manual        Run in manual/interactive mode (default)
  --help          Show help message
```

## Test Execution Flow

1. **Register tests** via `RegisterSimpleGuiTests(engine)`
2. **Start engine** with `ImGuiTestEngine_Start()`
3. **Main loop**:
   - Render your GUI (GuiFunc called)
   - Test engine runs tests (TestFunc called)
   - Display Test Engine UI
4. **Stop engine** and cleanup

## Next Steps

Once you understand these simple tests, you can:

1. **Add more complex interactions**:
   - `ItemInput()` for text input
   - `ItemOpen()/ItemClose()` for tree nodes
   - `MenuClick()` for menu items

2. **Test your actual application**:
   - Replace simple GUI with your MarketDataTable
   - Test data updates and filtering
   - Verify table rendering

3. **Add assertions**:
   - Check specific values in your data
   - Verify UI state after operations
   - Test edge cases and error handling

## Common Patterns

### Pattern: Test a sequence of actions
```cpp
t->TestFunc = [](ImGuiTestContext* ctx) {
    ctx->SetRef("My Window");
    ctx->ItemClick("Button 1");
    ctx->ItemCheck("Enable Feature");
    ctx->ItemClick("Button 2");
    IM_CHECK_EQ(vars.Result, ExpectedValue);
};
```

### Pattern: Test with delays
```cpp
t->TestFunc = [](ImGuiTestContext* ctx) {
    ctx->ItemClick("Start");
    ctx->Sleep(1.0f);  // Wait 1 second
    IM_CHECK(vars.IsComplete);
};
```

### Pattern: Iterate and verify
```cpp
t->TestFunc = [](ImGuiTestContext* ctx) {
    for (int i = 0; i < 5; i++) {
        ctx->ItemClick("Increment");
        IM_CHECK_EQ(vars.Counter, i + 1);
    }
};
```

## Tips

- **Start simple**: Get basic tests working before adding complexity
- **Use descriptive names**: Make test names clear ("button_click" not "test2")
- **One concept per test**: Keep tests focused on one feature
- **Check incrementally**: Add assertions after each action
- **Use the Test Engine UI**: It shows what's happening in real-time

## References

- [ImGui Test Engine Documentation](https://github.com/ocornut/imgui_test_engine)
- `imgui_test_engine/app_minimal/` - Official minimal example
- `imgui_test_engine/imgui_test_suite/` - Comprehensive test suite examples
