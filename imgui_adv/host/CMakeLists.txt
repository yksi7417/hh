
add_executable(host_demo main.cpp)
target_include_directories(host_demo PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)
if (WIN32)
  target_link_libraries(host_demo PRIVATE ws2_32)
  # Copy md_plugin.dll to the same directory as the executable
  add_custom_command(TARGET host_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:md_plugin>
    $<TARGET_FILE_DIR:host_demo>)
else()
  target_link_libraries(host_demo PRIVATE dl pthread)
endif()

if (WITH_IMGUI)
  include(FetchContent)

  # GLFW
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
  )
  FetchContent_MakeAvailable(glfw)

  # Dear ImGui
  FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.9
  )
  FetchContent_GetProperties(imgui)
  if (NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
  endif()

  set(IMGUI_DIR ${imgui_SOURCE_DIR})
  set(IMGUI_BACKENDS ${IMGUI_DIR}/backends)
  add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_BACKENDS}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS}/imgui_impl_opengl3.cpp
  )
  target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_BACKENDS}
  )
  target_link_libraries(imgui PUBLIC glfw)

  # GUI host
  add_executable(host_imgui imgui_host.cpp)
  target_include_directories(host_imgui PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)
  target_link_libraries(host_imgui PRIVATE imgui glfw)
  if (APPLE)
    target_link_libraries(host_imgui PRIVATE "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" "-framework OpenGL")
  elseif (WIN32)
    target_link_libraries(host_imgui PRIVATE opengl32)
    # Copy md_plugin.dll to the same directory as the executable
    add_custom_command(TARGET host_imgui POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:md_plugin>
      $<TARGET_FILE_DIR:host_imgui>)
  else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(host_imgui PRIVATE OpenGL::GL dl pthread)
  endif()
endif()
