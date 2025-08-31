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
target_include_directories(emsp PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)

add_custom_command(TARGET emsp POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:md_plugin>
      $<TARGET_FILE_DIR:emsp>)

find_package(glad CONFIG REQUIRED)
target_link_libraries(emsp PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(emsp PRIVATE glfw)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(emsp PRIVATE imgui::imgui)

add_subdirectory(plugins)  
