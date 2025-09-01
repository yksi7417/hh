@echo off
echo Building EMSP project...

cd /d "%~dp0"

echo.
echo Installing dependencies from root project...
cd c:\dvlp\hh
vcpkg install --triplet x64-windows

echo.
echo Configuring CMake with proper vcpkg toolchain...
cd c:\dvlp\hh\imgui_opengl_glad
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=c:/dvlp/hh/vcpkg_installed/x64-windows/share/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_TESTS=OFF -DWITH_IMGUI=ON

echo.
echo Building project...
cmake --build build --config Debug

echo.
echo Build completed!
pause