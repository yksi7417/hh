@echo off
echo Simple build script without vcpkg dependencies...

cd /d "%~dp0"

echo.
echo Note: This script builds without external GUI dependencies
echo For full GUI build, use build_and_test.bat or set up vcpkg properly

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
