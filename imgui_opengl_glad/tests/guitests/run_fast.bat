@echo off
echo Running Simple GUI Tests in Fast Mode (Interactive)...
echo.
build\Debug\simple_gui_test.exe --fast
echo.
echo Exit code: %ERRORLEVEL%
