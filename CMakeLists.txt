cmake_minimum_required(VERSION 3.14)
project(cses_autograder CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Core Library (Logic)
add_library(autograder_core
    src/compiler.cpp
    src/runner.cpp
    src/validator.cpp
)
target_include_directories(autograder_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Main Executable
add_executable(cses-grader src/main.cpp)
target_link_libraries(cses-grader PRIVATE autograder_core)

# Tests
enable_testing()

add_executable(autograder_tests
    tests/test_validator.cpp
    tests/test_compiler.cpp
    tests/test_runner.cpp
)
target_link_libraries(autograder_tests
    PRIVATE
    autograder_core
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(autograder_tests)
