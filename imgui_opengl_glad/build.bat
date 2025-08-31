vcpkg install --triplet x64-windows --x-install-root=C:\dvlp\vcpkg\installed

cmake -B .\build\ -DCMAKE_TOOLCHAIN_FILE=C:\dvlp\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build .\build\ --config Debug