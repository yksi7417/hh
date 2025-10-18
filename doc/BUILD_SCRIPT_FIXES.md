# Build Script Output Path Fixes

## Changes Made

All three build scripts (`build_ui_test.bat`, `build_tests_only.bat`, `build_gui.bat`) have been updated to show **absolute paths** in their output instead of relative paths.

## Problem

The build scripts change directories during execution (`cd imgui_opengl_glad` or `cd imgui_opengl_glad\tests\guitests`), which caused the output messages to show incorrect relative paths like:

```
✓ GUI test executable: .\build\Debug\simple_gui_test.exe
```

This path only works if you're already in the `imgui_opengl_glad\tests\guitests` directory, which is confusing for users who run the script from the root.

## Solution

Each script now:
1. Captures the absolute path using `set "VAR=%CD%"` after changing directories
2. Shows the full absolute path in output messages
3. Returns to the root directory at the end using `cd /d "%~dp0"`
4. Provides clear instructions with full paths

## Examples

### build_ui_test.bat
**Before:**
```
✓ GUI test executable: .\build\Debug\simple_gui_test.exe
```

**After:**
```
✓ GUI test executable: C:\dvlp\hh\imgui_opengl_glad\tests\guitests\build\Debug\simple_gui_test.exe

=== How to Run GUI Tests ===

Headless (automated, for CI/CD):
  cd C:\dvlp\hh\imgui_opengl_glad\tests\guitests
  .\build\Debug\simple_gui_test.exe --headless

Manual (interactive, for debugging):
  cd C:\dvlp\hh\imgui_opengl_glad\tests\guitests
  .\build\Debug\simple_gui_test.exe --manual
```

### build_tests_only.bat
**Before:**
```
Test executable: .\build_tests\Debug\unit_tests.exe
```

**After:**
```
Test executable: C:\dvlp\hh\imgui_opengl_glad\build_tests\Debug\unit_tests.exe

To run tests again:
  cd C:\dvlp\hh\imgui_opengl_glad\build_tests
  ctest --build-config Debug --verbose

Or run directly:
  C:\dvlp\hh\imgui_opengl_glad\build_tests\Debug\unit_tests.exe
```

### build_gui.bat
**Before:**
```
✓ GUI executable: .\build_gui\Debug\emsp.exe

You can run the application with: .\build_gui\Debug\emsp.exe
```

**After:**
```
✓ GUI executable: C:\dvlp\hh\imgui_opengl_glad\build_gui\Debug\emsp.exe

You can run the application with:
  cd C:\dvlp\hh\imgui_opengl_glad\build_gui\Debug
  .\emsp.exe

Or directly:
  C:\dvlp\hh\imgui_opengl_glad\build_gui\Debug\emsp.exe
```

## Benefits

✅ **Clear paths**: Users can copy-paste the exact path to the executable  
✅ **No confusion**: Absolute paths work from any directory  
✅ **Better UX**: Clear instructions on how to run the built executables  
✅ **Consistent**: All three build scripts now follow the same pattern  
✅ **Returns to root**: Scripts return to the starting directory at the end

## Technical Details

Each script uses:
```batch
REM Capture absolute path after cd
set "VAR_NAME=%CD%"

REM Use in output
echo Path: %VAR_NAME%\build\...

REM Return to root at end
cd /d "%~dp0"
```

This ensures the script:
1. Shows correct absolute paths
2. Handles errors by returning to root before exit
3. Leaves the user in the root directory after completion
