
add_library(md_plugin SHARED md_plugin.cpp)
target_include_directories(md_plugin PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)

# Threads for simulator
find_package(Threads REQUIRED)
target_link_libraries(md_plugin PRIVATE Threads::Threads)
set_target_properties(md_plugin PROPERTIES OUTPUT_NAME "md_plugin")
