cmake_minimum_required(VERSION 3.16)
project(SimpleGuiTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ImGui source files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
)

# We'll compile imgui_impl_opengl3.cpp using the custom wrapper
# to ensure GLAD is included properly
set(IMGUI_OPENGL3_WRAPPER ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui_opengl3_glad.cpp)

# ImGui Test Engine source files
set(IMGUI_TEST_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui_test_engine/imgui_test_engine)
set(IMGUI_TEST_ENGINE_SOURCES
    ${IMGUI_TEST_ENGINE_DIR}/imgui_capture_tool.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_context.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_coroutine.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_engine.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_exporters.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_perftool.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_ui.cpp
    ${IMGUI_TEST_ENGINE_DIR}/imgui_te_utils.cpp
)

# Application source files (for testing the real GUI)
set(APP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(APP_SOURCES
    ${APP_DIR}/ui/IMGuiComponents.cpp
    ${APP_DIR}/ui/MarketDataTable.cpp
    ${APP_DIR}/core/data_updater.cpp
)

# Simple GUI test executable
add_executable(simple_gui_test 
    simple_gui_test_main.cpp
    simple_gui_test.cpp
    app_gui_test.cpp
    ${IMGUI_SOURCES}
    ${IMGUI_OPENGL3_WRAPPER}
    ${IMGUI_TEST_ENGINE_SOURCES}
    ${APP_SOURCES}
)

# Compiler definitions
target_compile_definitions(simple_gui_test PRIVATE 
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM
    IMGUI_IMPL_OPENGL_LOADER_GLAD
    IMGUI_ENABLE_TEST_ENGINE
    IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1
    IMGUI_TEST_ENGINE_ENABLE_STD_FUNCTION=1
)

# Include directories
target_include_directories(simple_gui_test PRIVATE 
    ${IMGUI_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui_test_engine
    ${APP_DIR}
    ${APP_DIR}/core
    ${APP_DIR}/ui
)

# Find and link vcpkg packages
find_package(glad CONFIG REQUIRED)
target_link_libraries(simple_gui_test PRIVATE glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(simple_gui_test PRIVATE glfw)

# Windows-specific settings
if(MSVC)
    target_compile_options(simple_gui_test PRIVATE /W3)
    # Set as GUI application (not console)
    # set_target_properties(simple_gui_test PROPERTIES WIN32_EXECUTABLE TRUE)
else()
    target_compile_options(simple_gui_test PRIVATE -Wall -Wextra)
endif()
