@echo off
echo Running Simple GUI Test in Manual Mode...
echo.
build\Debug\simple_gui_test.exe --manual
echo.
echo Exit code: %ERRORLEVEL%
pause
