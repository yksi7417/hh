@echo off
echo Building EMSP GUI application with vcpkg dependencies...

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

REM Check for vcpkg
if not exist "c:\dvlp\vcpkg\vcpkg.exe" (
    echo ERROR: vcpkg not found at c:\dvlp\vcpkg\vcpkg.exe
    echo Please install vcpkg or update the path in this script
    exit /b 1
) else (
    echo ✓ vcpkg detected
)

echo All prerequisites satisfied!

echo.
echo Installing GUI dependencies with vcpkg...
c:\dvlp\vcpkg\vcpkg.exe install --triplet x64-windows
if %ERRORLEVEL% neq 0 (
    echo vcpkg install failed!
    exit /b 1
)

echo.
echo Configuring CMake for GUI build...
cd imgui_opengl_glad
cmake -B build_gui -S . ^
    -DCMAKE_TOOLCHAIN_FILE=c:/dvlp/vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DBUILD_TESTS=OFF ^
    -DWITH_IMGUI=ON
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

echo.
echo Building GUI application...
cmake --build build_gui --config Debug
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo GUI build completed!

REM Check if executable was actually created
if exist ".\build_gui\Debug\emsp.exe" (
    echo ✓ GUI executable: .\build_gui\Debug\emsp.exe
    echo.
    echo You can run the application with: .\build_gui\Debug\emsp.exe
) else (
    echo ✗ GUI executable NOT found: .\build_gui\Debug\emsp.exe
    echo The GUI build failed. Check the build output above.
    exit /b 1
)
