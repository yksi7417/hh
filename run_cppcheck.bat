@echo off
REM CppCheck static analysis script for the project
REM This script runs cppcheck on the main source directories

echo Running CppCheck static analysis...
echo.

REM Check if cppcheck is available
cppcheck --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: cppcheck not found in PATH
    echo Please install cppcheck first:
    echo   - winget install Cppcheck.Cppcheck
    echo   - OR choco install cppcheck  
    echo   - OR download from: https://github.com/danmar/cppcheck/releases
    echo.
    echo After installation, you may need to restart your terminal or add to PATH:
    echo   set PATH=%%PATH%%;C:\Program Files\Cppcheck
    echo.
    pause
    exit /b 1
)

echo CppCheck version:
cppcheck --version
echo.

REM Run cppcheck on main source directories
echo Analyzing source code...
cppcheck --enable=warning,performance,portability,information --error-exitcode=1 ^
    --suppress=missingIncludeSystem ^
    --suppress=unusedFunction ^
    --inline-suppr ^
    --std=c++17 ^
    --platform=win64 ^
    --template="{file}({line}): {severity}: {message} [{id}]" ^
    imgui_basic/ imgui_opengl_glad/core/ imgui_opengl_glad/ui/ imgui_opengl_glad/plugins/ ^
    --exclude=imgui_basic/imgui/ ^
    --exclude=imgui_basic/imgui_test_engine/ ^
    --exclude=imgui_opengl_glad/imgui/ ^
    --exclude=imgui_opengl_glad/imgui_test_engine/ ^
    --exclude=build/ ^
    --exclude=build_gui/ ^
    --exclude=build_tests/ ^
    --exclude=vcpkg_installed/

if errorlevel 1 (
    echo.
    echo CppCheck found issues that need attention.
    echo Please review and fix the reported problems.
    pause
    exit /b 1
) else (
    echo.
    echo CppCheck analysis completed successfully - no issues found!
    pause
)
