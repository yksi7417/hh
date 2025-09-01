@echo off
echo Running EMSP Code Quality and Design Principle Linter...
echo.

REM Check if Python is available
python --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: Python is required to run the design linter
    echo Please install Python or ensure it's in your PATH
    exit /b 1
)

echo [1/2] Checking design principles...
REM Run the custom design linter
python lint_design.py %*
set LINT_EXIT_CODE=%ERRORLEVEL%

if %LINT_EXIT_CODE% neq 0 (
    echo.
    echo ❌ Design principles violations found. Please fix or document justifications.
    echo.
    echo Justification examples:
    echo   // Performance critical: tight loop for market data processing
    echo   /// @brief Legacy API compatibility requires raw pointer
    echo   /** NOTE: Manual memory management needed for plugin interface */
    echo.
    set HAS_ERRORS=1
) else (
    echo ✅ Design principles check passed!
)

REM Run clang-tidy if available
echo.
echo [2/2] Running clang-tidy static analysis...
clang-tidy --version >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo Running clang-tidy on core and ui files...
    
    REM Check if compile_commands.json exists
    if exist "imgui_opengl_glad\build_tests\compile_commands.json" (
        clang-tidy -p imgui_opengl_glad\build_tests imgui_opengl_glad\core\*.cpp imgui_opengl_glad\ui\*.cpp --format-style=file
        if %ERRORLEVEL% neq 0 set HAS_ERRORS=1
    ) else if exist "imgui_opengl_glad\build_gui\compile_commands.json" (
        clang-tidy -p imgui_opengl_glad\build_gui imgui_opengl_glad\core\*.cpp imgui_opengl_glad\ui\*.cpp --format-style=file  
        if %ERRORLEVEL% neq 0 set HAS_ERRORS=1
    ) else (
        echo Warning: No compile_commands.json found. Run cmake configure first.
        echo Skipping clang-tidy analysis.
    )
) else (
    echo clang-tidy not found, skipping static analysis
    echo Install LLVM/Clang tools for additional checks
)

echo.
if defined HAS_ERRORS (
    echo ❌ Code quality checks found issues!
    exit /b 1
) else (
    echo ✅ All code quality checks passed!
    exit /b 0
)
