@echo off
echo Running Simple GUI Tests in Cinematic Mode (Slow, for demos)...
echo.
build\Debug\simple_gui_test.exe --cinematic
echo.
echo Exit code: %ERRORLEVEL%
