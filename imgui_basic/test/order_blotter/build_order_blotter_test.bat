@echo off
setlocal enabledelayedexpansion

echo Building OrderBlotter Test Suite...

:: Set up environment variables
set "BUILD_DIR=build"
set "ROOT_DIR=%~dp0..\.."
set "TEST_DIR=%~dp0"

:: Create build directory if it doesn't exist
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: Set compiler flags
set "CFLAGS=/std:c++17 /EHsc /MD /O2 /W3"
set "INCLUDES=/I"%ROOT_DIR%" /I"%ROOT_DIR%\imgui" /I"%ROOT_DIR%\imgui_test_engine" /I"%ROOT_DIR%\imgui_test_engine\thirdparty""
set "DEFINES=/DWIN32 /D_WINDOWS /DIMGUI_ENABLE_TEST_ENGINE"
set "LIBS=user32.lib gdi32.lib shell32.lib d3d11.lib d3dcompiler.lib"

:: Compile source files to object files
echo Compiling source files...

:: ImGui core files
cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui\imgui.cpp" /Fo"%BUILD_DIR%\imgui.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui\imgui_draw.cpp" /Fo"%BUILD_DIR%\imgui_draw.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui\imgui_tables.cpp" /Fo"%BUILD_DIR%\imgui_tables.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui\imgui_widgets.cpp" /Fo"%BUILD_DIR%\imgui_widgets.obj"
if errorlevel 1 goto :error

:: ImGui backends
cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui\imgui_impl_win32.cpp" /Fo"%BUILD_DIR%\imgui_impl_win32.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui\imgui_impl_dx11.cpp" /Fo"%BUILD_DIR%\imgui_impl_dx11.obj"
if errorlevel 1 goto :error

:: ImGui Test Engine files
cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui_test_engine\imgui_te_context.cpp" /Fo"%BUILD_DIR%\imgui_te_context.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui_test_engine\imgui_te_coroutine.cpp" /Fo"%BUILD_DIR%\imgui_te_coroutine.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui_test_engine\imgui_te_engine.cpp" /Fo"%BUILD_DIR%\imgui_te_engine.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui_test_engine\imgui_te_exporters.cpp" /Fo"%BUILD_DIR%\imgui_te_exporters.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui_test_engine\imgui_te_perftool.cpp" /Fo"%BUILD_DIR%\imgui_te_perftool.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui_test_engine\imgui_te_ui.cpp" /Fo"%BUILD_DIR%\imgui_te_ui.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui_test_engine\imgui_te_utils.cpp" /Fo"%BUILD_DIR%\imgui_te_utils.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\imgui_test_engine\imgui_capture_tool.cpp" /Fo"%BUILD_DIR%\imgui_capture_tool.obj"
if errorlevel 1 goto :error

:: OrderBlotter source
cl %CFLAGS% %INCLUDES% %DEFINES% /c "%ROOT_DIR%\order_blotter.cpp" /Fo"%BUILD_DIR%\order_blotter.obj"
if errorlevel 1 goto :error

:: Test files
cl %CFLAGS% %INCLUDES% %DEFINES% /c "%TEST_DIR%order_blotter_test_helper.cpp" /Fo"%BUILD_DIR%\order_blotter_test_helper.obj"
if errorlevel 1 goto :error

cl %CFLAGS% %INCLUDES% %DEFINES% /c "%TEST_DIR%order_blotter_test.cpp" /Fo"%BUILD_DIR%\order_blotter_test.obj"
if errorlevel 1 goto :error

:: Link the executable
echo Linking executable...
link /OUT:"%BUILD_DIR%\order_blotter_test.exe" ^
     "%BUILD_DIR%\imgui.obj" ^
     "%BUILD_DIR%\imgui_draw.obj" ^
     "%BUILD_DIR%\imgui_tables.obj" ^
     "%BUILD_DIR%\imgui_widgets.obj" ^
     "%BUILD_DIR%\imgui_impl_win32.obj" ^
     "%BUILD_DIR%\imgui_impl_dx11.obj" ^
     "%BUILD_DIR%\imgui_te_context.obj" ^
     "%BUILD_DIR%\imgui_te_coroutine.obj" ^
     "%BUILD_DIR%\imgui_te_engine.obj" ^
     "%BUILD_DIR%\imgui_te_exporters.obj" ^
     "%BUILD_DIR%\imgui_te_perftool.obj" ^
     "%BUILD_DIR%\imgui_te_ui.obj" ^
     "%BUILD_DIR%\imgui_te_utils.obj" ^
     "%BUILD_DIR%\imgui_capture_tool.obj" ^
     "%BUILD_DIR%\order_blotter.obj" ^
     "%BUILD_DIR%\order_blotter_test_helper.obj" ^
     "%BUILD_DIR%\order_blotter_test.obj" ^
     %LIBS%

if errorlevel 1 goto :error

echo.
echo Build completed successfully!
echo Executable: %BUILD_DIR%\order_blotter_test.exe
goto :end

:error
echo.
echo Build failed with error!
exit /b 1

:end
endlocal
