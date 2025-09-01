@echo off
echo Building EMSP project with GTest unit tests...

cd /d "%~dp0"

echo.
echo Configuring CMake with tests enabled...
cmake -B build -S . -DBUILD_TESTS=ON
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building project...
cmake --build build --config Debug
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Running unit tests with CTest...
cd build
ctest --build-config Debug --verbose --output-on-failure
if %ERRORLEVEL% neq 0 (
    echo Unit tests failed!
    cd ..
    pause
    exit /b 1
)
cd ..

echo.
echo All builds and tests completed successfully!
echo Main executable: .\build\Debug\emsp.exe
echo GTest executable: .\build\tests\Debug\unit_tests.exe
pause
