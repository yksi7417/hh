@echo off
echo Simple build script without vcpkg dependencies...

cd /d "%~dp0"

echo.
echo Checking build prerequisites...

REM Check for PowerShell Core
pwsh.exe --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: PowerShell Core ^(pwsh.exe^) is required but not found!
    echo Please install with: winget install Microsoft.PowerShell
    echo Why needed: Google Test's test discovery and CTest operations require PowerShell Core
    pause
    exit /b 1
) else (
    echo ✓ PowerShell Core detected
)

echo All prerequisites satisfied!

echo.
echo Note: This script builds without external GUI dependencies
echo For full GUI build, use build.bat or set up vcpkg properly

echo.
echo Configuring CMake (tests only)...
cmake -B build -S . -DBUILD_TESTS=ON -DWITH_IMGUI=OFF

echo.
echo Building tests...
cmake --build build --config Debug --target unit_tests

echo.
echo Running tests...
cd build
ctest --build-config Debug --verbose

echo.
echo Test build completed!
echo Test executable: .\build\tests\Debug\unit_tests.exe
pause
