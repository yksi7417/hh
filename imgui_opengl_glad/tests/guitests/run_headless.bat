@echo off
echo Running Simple GUI Tests in Headless Mode...
echo.
build\Debug\simple_gui_test.exe --headless
echo.
if %ERRORLEVEL% EQU 0 (
    echo SUCCESS: All tests passed!
) else (
    echo FAILURE: Some tests failed!
)
echo Exit code: %ERRORLEVEL%
exit /b %ERRORLEVEL%
