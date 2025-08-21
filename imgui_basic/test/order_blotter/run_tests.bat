@echo off
setlocal enabledelayedexpansion

echo OrderBlotter Test Runner
echo ======================

:: Check if executable exists
if not exist "build\order_blotter_test.exe" (
    echo Test executable not found. Building...
    call build_order_blotter_test.bat
    if errorlevel 1 (
        echo Build failed! Cannot run tests.
        exit /b 1
    )
)

:: Run the tests
echo.
echo Running OrderBlotter tests...
echo.

pushd build
order_blotter_test.exe
set TEST_RESULT=%errorlevel%
popd

echo.
echo ======================
if %TEST_RESULT% == 0 (
    echo All tests PASSED! ✅
) else (
    echo Some tests FAILED! ❌
)
echo Test execution completed.
echo ======================

exit /b %TEST_RESULT%
