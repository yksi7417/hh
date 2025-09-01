@echo off
echo Building EMSP project with GUI and optional tests...

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

REM Check for CMake
cmake --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake is required but not found!
    pause
    exit /b 1
) else (
    echo ✓ CMake detected
)

echo All prerequisites satisfied!

echo.
echo Installing dependencies from root project...
cd c:\dvlp\hh
vcpkg install --triplet x64-windows

echo.
echo Configuring CMake with proper vcpkg toolchain...
cd c:\dvlp\hh\imgui_opengl_glad
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=c:/dvlp/hh/vcpkg_installed/x64-windows/share/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_TESTS=ON -DWITH_IMGUI=ON

echo.
echo Building project...
cmake --build build --config Debug

echo.
echo Running tests...
cd build
ctest --build-config Debug --output-on-failure
cd ..

echo.
echo Build completed successfully!
echo Main executable: .\build\Debug\emsp.exe
echo Test executable: .\build\tests\Debug\unit_tests.exe
pause