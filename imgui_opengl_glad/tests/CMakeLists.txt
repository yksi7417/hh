cmake_minimum_required(VERSION 3.16)

# Include CTest for running tests
include(CTest)
enable_testing()

# Use FetchContent to download GTest
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the unit test executable
add_executable(unit_tests
    unittests/simple_test.cpp
    unittests/test_data_updater.cpp
    ../data_updater.cpp
)

# Set up include directories
target_include_directories(unit_tests PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}/../../include
    ${CMAKE_CURRENT_LIST_DIR}/..
)

# Link against GTest using FetchContent
target_link_libraries(unit_tests 
    PRIVATE 
    gtest_main
)

# Set C++ standard
target_compile_features(unit_tests PRIVATE cxx_std_17)

# Add the test to CTest
add_test(NAME DataUpdaterUnitTests COMMAND unit_tests)

# Set test properties
set_tests_properties(DataUpdaterUnitTests 
    PROPERTIES 
    TIMEOUT 30
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Optional: Add additional test configurations
if(WIN32)
    # Windows-specific test settings
    set_tests_properties(DataUpdaterUnitTests 
        PROPERTIES 
        ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/Debug;$ENV{PATH}"
    )
endif()
