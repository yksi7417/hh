@echo off
echo Simple build script without vcpkg dependencies...

cd /d "%~dp0"

echo.
echo Checking build prerequisites...

REM Check for PowerShell Core first, then Windows PowerShell
pwsh.exe --version >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo PowerShell Core detected
    set POWERSHELL_CMD=pwsh.exe
    goto :powershell_found
)

powershell.exe -Command "exit 0" >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo Windows PowerShell detected ^(PowerShell Core preferred but this will work^)
    set POWERSHELL_CMD=powershell.exe
    goto :powershell_found
)

echo ERROR: No PowerShell found! Either PowerShell Core ^(pwsh.exe^) or Windows PowerShell ^(powershell.exe^) is required
echo For best compatibility, install PowerShell Core with: winget install Microsoft.PowerShell
echo Why needed: Google Test's test discovery and CTest operations require PowerShell
exit /b 1

:powershell_found
echo All prerequisites satisfied!

echo.
echo Note: This script builds without external GUI dependencies
echo For full GUI build, use build.bat or set up vcpkg properly

echo.
echo Configuring CMake (tests only)...
cd imgui_opengl_glad
cmake -B build_tests -S . -DBUILD_TESTS=ON -DWITH_IMGUI=OFF

echo.
echo Building tests...
cmake --build build_tests --config Debug --target unit_tests

echo.
echo Running tests...
cd build_tests
ctest --build-config Debug --verbose

REM Save the absolute path to the tests subdirectory
set "TEST_DIR=%CD%\tests"

echo.
echo Test build completed!
echo Test executable: %TEST_DIR%\Debug\unit_tests.exe
echo.
echo To run tests again:
echo   cd %TEST_DIR%\..
echo   ctest --build-config Debug --verbose
echo.
echo Or run directly:
echo   %TEST_DIR%\Debug\unit_tests.exe

cd /d "%~dp0"
