@echo off
echo Building GUI Tests (ImGui Test Engine)...

cd /d "%~dp0"

echo.
echo Checking build prerequisites...

REM Check for PowerShell Core
pwsh.exe --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: PowerShell Core ^(pwsh.exe^) is required but not found!
    echo Please install with: winget install Microsoft.PowerShell
    exit /b 1
) else (
    echo ✓ PowerShell Core detected
)

REM Check for CMake
cmake --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake is required but not found!
    exit /b 1
) else (
    echo ✓ CMake detected
)

echo All prerequisites satisfied!

echo.
echo Note: GUI tests use ImGui Test Engine to test UI components
echo For unit tests only, use build_tests_only.bat
echo For full GUI application, use build_gui.bat

echo.
echo Configuring CMake for GUI tests...
cd imgui_opengl_glad\tests\guitests
cmake -B build -S .
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    cd /d "%~dp0"
    exit /b 1
)

echo.
echo Building GUI tests...
cmake --build build --config Debug
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    cd /d "%~dp0"
    exit /b 1
)

echo.
echo GUI test build completed!

REM Save the absolute path before checking
set "GUI_TEST_DIR=%CD%"

REM Check if executable was actually created
if exist ".\build\Debug\simple_gui_test.exe" (
    echo ✓ GUI test executable: %GUI_TEST_DIR%\build\Debug\simple_gui_test.exe
    echo.
    echo === How to Run GUI Tests ===
    echo.
    echo Headless ^(automated, for CI/CD^):
    echo   cd %GUI_TEST_DIR%
    echo   .\build\Debug\simple_gui_test.exe --headless
    echo.
    echo Manual ^(interactive, for debugging^):
    echo   cd %GUI_TEST_DIR%
    echo   .\build\Debug\simple_gui_test.exe --manual
    echo.
    echo Help:
    echo   .\build\Debug\simple_gui_test.exe --help
    echo.
    echo See %GUI_TEST_DIR%\QUICK_REFERENCE.md for more details
) else (
    echo ✗ GUI test executable NOT found: %GUI_TEST_DIR%\build\Debug\simple_gui_test.exe
    echo The GUI test build failed. Check the build output above.
    cd /d "%~dp0"
    exit /b 1
)

cd /d "%~dp0"
