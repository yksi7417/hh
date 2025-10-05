@echo off
REM Build script for Simple GUI Tests using CMake

echo Building Simple GUI Tests with CMake...
echo.

REM Check if vcpkg toolchain file exists
set VCPKG_TOOLCHAIN=c:\dvlp\vcpkg\scripts\buildsystems\vcpkg.cmake
if not exist "%VCPKG_TOOLCHAIN%" (
    echo ERROR: vcpkg toolchain file not found at: %VCPKG_TOOLCHAIN%
    echo Please install vcpkg or update the path in this script
    exit /b 1
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure CMake
echo Configuring CMake...
cmake -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN% ^
    ..

if errorlevel 1 (
    echo.
    echo CMake configuration failed!
    cd ..
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config Debug

if errorlevel 1 (
    echo.
    echo Build failed!
    cd ..
    exit /b 1
)

echo.
echo Build successful!
echo Executable: build\Debug\simple_gui_test.exe
echo.
echo To run the tests, execute:
echo   cd build\Debug
echo   simple_gui_test.exe
echo.

cd ..
