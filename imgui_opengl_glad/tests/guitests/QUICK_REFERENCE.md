# GUI Testing Quick Reference

## Run Tests

### Headless (CI/CD)
```bash
cd c:\dvlp\hh\imgui_opengl_glad\tests\guitests
.\build\Debug\simple_gui_test.exe --headless
```
Exit code 0 = all passed, non-zero = failures

### Manual (Interactive Debug)
```bash
.\build\Debug\simple_gui_test.exe --manual
```
Opens GUI with Test Engine controls

### Help
```bash
.\build\Debug\simple_gui_test.exe --help
```

## Build Tests
```bash
cmake --build build --config Debug
```

## Current Tests (6 total)

### Simple Examples
1. `simple_gui/basic_window`
2. `simple_gui/button_click`
3. `simple_gui/checkbox`

### Application Tests
4. `app_gui/market_data_table_window`
5. `app_gui/market_data_table_has_rows`
6. `app_gui/data_updates`

## Add New Test

Edit `app_gui_test.cpp`:

```cpp
void RegisterAppGuiTests(ImGuiTestEngine* engine) {
    // ... existing tests ...
    
    // Your new test
    ImGuiTest* t = IM_REGISTER_TEST(engine, "app_gui", "my_new_test");
    t->SetVarsDataType<AppGuiTestVars>();
    
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        AppGuiTestVars& vars = ctx->GetVars<AppGuiTestVars>();
        vars.Initialize();
        
        // Render your GUI
        vars.market_data_table->UpdateFromContext(vars.ctx, vars.slot, should_refresh);
        vars.market_data_table->Render(vars.ctx, vars.slot);
    };
    
    t->TestFunc = [](ImGuiTestContext* ctx) {
        // Verify expected behavior
        ctx->SetRef("MarketData");
        IM_CHECK(ctx->GetWindowByRef("") != nullptr);
        
        // Add more checks here
    };
}
```

Rebuild and run!

## Files

- `app_gui_test.cpp` - Application GUI tests
- `simple_gui_test.cpp` - Basic example tests
- `simple_gui_test_main.cpp` - Test runner
- `CMakeLists.txt` - Build configuration
- `APP_TESTING.md` - Detailed documentation
- `TESTING_SUCCESS.md` - How everything works

## Tips

1. **Window name**: Use `"MarketData"` not "Market Data Table"
2. **Mock data**: Set test data in `AppGuiTestVars::Initialize()`
3. **Assertions**: Use `IM_CHECK()` for test verification
4. **Sleep**: Use `ctx->Sleep(seconds)` not `Yield()`
5. **Manual mode**: Best for debugging failing tests

## Status

✅ **All 6 tests passing**  
✅ **Ready for CI/CD**  
✅ **Application components tested**
