
cmake_minimum_required(VERSION 3.16)
project(soa_md_modelA_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WITH_IMGUI "Build GUI host using Dear ImGui + GLFW + OpenGL3" ON)

if (MSVC)
  add_compile_options(/W3)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(host)
add_subdirectory(plugin)
