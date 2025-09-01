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

# ImGui source files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_opengl3_glad.cpp
)

# ImGui Test Engine source files
set(IMGUI_TEST_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui_test_engine/imgui_test_engine)
set(IMGUI_TEST_ENGINE_SOURCES
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_context.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_coroutine.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_engine.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_exporters.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_perftool.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_ui.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_utils.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_capture_tool.cpp
)

add_executable (emsp "main.cpp" "IMGuiComponents.cpp" "IMGuiComponents.h" ${IMGUI_SOURCES})

target_compile_definitions(emsp PRIVATE 
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM
    IMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_include_directories(emsp PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}/../include
    ${IMGUI_DIR}
)

add_custom_command(TARGET emsp POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:md_plugin>
      $<TARGET_FILE_DIR:emsp>)

find_package(glad CONFIG REQUIRED)
target_link_libraries(emsp PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(emsp PRIVATE glfw)

# Remove imgui from vcpkg dependencies since we're using local version
# find_package(imgui CONFIG REQUIRED)
# target_link_libraries(emsp PRIVATE imgui::imgui)

add_subdirectory(plugins)
