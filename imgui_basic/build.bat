@echo off
setlocal

rem Use the VS "x64 Native Tools Command Prompt" so env is set.
set OUT=build
if not exist %OUT% mkdir %OUT%
pushd %OUT%

cl /nologo /Zi /W4 /EHsc ^
    ..\main.cpp ^
    ..\imgui\imgui.cpp ^
    ..\imgui\imgui_draw.cpp ^
    ..\imgui\imgui_widgets.cpp ^
    ..\imgui\imgui_tables.cpp ^
    ..\imgui\imgui_impl_win32.cpp ^
    ..\imgui\imgui_impl_dx11.cpp ^
    /I..\imgui ^
    user32.lib gdi32.lib d3d11.lib dxgi.lib

if %errorlevel% neq 0 exit /b %errorlevel%
echo Build complete.
