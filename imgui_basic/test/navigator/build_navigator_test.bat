@echo off
setlocal

rem Use the VS "x64 Native Tools Command Prompt" so env is set.
set OUT=build
if not exist %OUT% mkdir %OUT%

rem Build Navigator test application
pushd %OUT%
echo Building Navigator test application...

rem Set common compile flags
set CFLAGS=/nologo /Zi /W4 /EHsc /std:c++17 /DIMGUI_ENABLE_TEST_ENGINE
set INCLUDES=/I..\..\..\imgui /I..\..\..\imgui_test_engine /I..\..\..

echo Compiling source files...

rem Compile test files
cl %CFLAGS% %INCLUDES% /c ..\navigator_test.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\navigator_test_helper.cpp
if %errorlevel% neq 0 goto :error

rem Compile navigator
cl %CFLAGS% %INCLUDES% /c ..\..\..\navigator.cpp
if %errorlevel% neq 0 goto :error

rem Compile imgui files
cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui\imgui.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui\imgui_draw.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui\imgui_widgets.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui\imgui_tables.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui\imgui_impl_win32.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui\imgui_impl_dx11.cpp
if %errorlevel% neq 0 goto :error

rem Compile test engine files
cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui_test_engine\imgui_te_engine.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui_test_engine\imgui_te_context.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui_test_engine\imgui_te_coroutine.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui_test_engine\imgui_te_ui.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui_test_engine\imgui_te_utils.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui_test_engine\imgui_te_exporters.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui_test_engine\imgui_te_perftool.cpp
if %errorlevel% neq 0 goto :error

cl %CFLAGS% %INCLUDES% /c ..\..\..\imgui_test_engine\imgui_capture_tool.cpp
if %errorlevel% neq 0 goto :error

echo Linking...
link /nologo /debug *.obj user32.lib gdi32.lib d3d11.lib dxgi.lib /out:navigator_test.exe
if %errorlevel% neq 0 goto :error

popd
echo Navigator test build complete.
echo Run: build\navigator_test.exe
goto :end

:error
echo Navigator test build failed!
popd
exit /b %errorlevel%

:end
