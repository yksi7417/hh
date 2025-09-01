@echo off
echo Starting Test Suite...
echo.

rem Try to find and call VS Developer Command Prompt setup
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo Setting up Visual Studio 2022 Community environment...
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    echo Setting up Visual Studio 2022 Professional environment...
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    echo Setting up Visual Studio 2022 Enterprise environment...
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo Warning: Could not find Visual Studio 2022 installation.
    echo Please run this from "x64 Native Tools Command Prompt for VS"
    echo.
)

echo Running Navigator tests...
cd navigator
call run_tests.bat
cd ..

echo.
echo All tests completed.
