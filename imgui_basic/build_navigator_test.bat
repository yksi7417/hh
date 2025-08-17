@echo off
setlocal

rem Use the VS "x64 Native Tools Command Prompt" so env is set.
set OUT=build
if not exist %OUT% mkdir %OUT%

rem Build Navigator test application
pushd %OUT%
echo Building Navigator test application...
cl /nologo /Zi /W4 /EHsc /std:c++17 ^
    /DIMGUI_ENABLE_TEST_ENGINE ^
    ..\navigator_test.cpp ^
    ..\navigator_test_helper.cpp ^
    ..\navigator.cpp ^
    ..\imgui\imgui.cpp ^
    ..\imgui\imgui_draw.cpp ^
    ..\imgui\imgui_widgets.cpp ^
    ..\imgui\imgui_tables.cpp ^
    ..\imgui\imgui_impl_win32.cpp ^
    ..\imgui\imgui_impl_dx11.cpp ^
    ..\imgui_test_engine\imgui_te_engine.cpp ^
    ..\imgui_test_engine\imgui_te_context.cpp ^
    ..\imgui_test_engine\imgui_te_coroutine.cpp ^
    ..\imgui_test_engine\imgui_te_ui.cpp ^
    ..\imgui_test_engine\imgui_te_utils.cpp ^
    ..\imgui_test_engine\imgui_te_exporters.cpp ^
    ..\imgui_test_engine\imgui_te_perftool.cpp ^
    ..\imgui_test_engine\imgui_capture_tool.cpp ^
    /I..\imgui ^
    /I..\imgui_test_engine ^
    /I.. ^
    /Fe:navigator_test.exe ^
    user32.lib gdi32.lib d3d11.lib dxgi.lib

if %errorlevel% neq 0 (
    echo Navigator test build failed!
    popd
    exit /b %errorlevel%
)
popd

echo Navigator test build complete.
echo Run: build\navigator_test.exe
