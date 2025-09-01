@echo off
REM Pre-commit test validation script
REM Builds and runs tests with minimal output for commit hook context

cd /d "%~dp0"

REM Quick prerequisite check - accept either PowerShell Core or Windows PowerShell
pwsh.exe --version >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set POWERSHELL_CMD=pwsh.exe
) else (
    powershell.exe -Command "exit 0" >nul 2>&1
    if %ERRORLEVEL% equ 0 (
        set POWERSHELL_CMD=powershell.exe
    ) else (
        echo ERROR: PowerShell required for tests
        exit /b 1
    )
)

echo Building and running tests...
cd imgui_opengl_glad

REM Configure tests (suppress verbose output)
cmake -B build_tests -S . -DBUILD_TESTS=ON -DWITH_IMGUI=OFF >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to configure test build
    exit /b 1
)

REM Build tests (suppress verbose output)
cmake --build build_tests --config Debug --target unit_tests >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to build tests
    exit /b 1
)

REM Run tests with minimal output
cd build_tests
echo Running unit tests...
ctest --build-config Debug --output-on-failure --parallel
if %ERRORLEVEL% neq 0 (
    echo ERROR: Some tests failed
    exit /b 1
)

echo All tests passed successfully
cd ..\..
exit /b 0
