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
    exit /b 1
)

echo CppCheck version:
cppcheck --version
echo.

REM Run cppcheck on main source directories
echo Analyzing source code...
echo.
echo Analyzing imgui_opengl_glad project files...
cppcheck --enable=warning,performance,portability --error-exitcode=1 ^
    --suppress=missingIncludeSystem ^
    --suppress=unusedFunction ^
    --suppress=preprocessorErrorDirective ^
    --suppress=normalCheckLevelMaxBranches ^
    --inline-suppr ^
    --std=c++17 ^
    --language=c++ ^
    --platform=win64 ^
    --template="{file}({line}): {severity}: {message} [{id}]" ^
    imgui_opengl_glad/core/ imgui_opengl_glad/ui/ imgui_opengl_glad/plugins/

if errorlevel 1 (
    echo.
    echo CppCheck found issues that need attention.
    echo Please review and fix the reported problems.
    exit /b 1
) else (
    echo.
    echo CppCheck analysis completed successfully - no issues found!
)
