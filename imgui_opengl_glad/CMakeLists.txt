cmake_minimum_required(VERSION 3.16)
project(EMSP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WITH_IMGUI "Build GUI EMSP Host using Dear ImGui + GLFW + OpenGL3" ON)

if (MSVC)
  add_compile_options(/W3)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable (emsp "main.cpp" "IMGuiComponentList.cpp" "IMGuiComponentList.h")

find_package(glad CONFIG REQUIRED)
target_link_libraries(emsp PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(emsp PRIVATE glfw)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(emsp PRIVATE imgui::imgui)

# add_subdirectory(plugins)  # Empty directory, commented out for now
