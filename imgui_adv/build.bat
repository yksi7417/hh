mkdir build 
cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DWITH_IMGUI=ON
cmake --build . --clean-first -j