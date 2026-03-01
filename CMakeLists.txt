cmake_minimum_required(VERSION 3.14)
project(EAE_Firmware VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Core Firmware Executable ---
# Gather all source files except tests
file(GLOB SOURCES "src/*.cpp")

add_executable(cooling_sys ${SOURCES})

# --- GTest Integration ---
# Download GTest automatically at configure time. Do NOT ship it in the repo.
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)
# For Windows/MSYS2 compatibility
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Unit Test Executable ---
enable_testing()

add_executable(run_tests tests/test_pid.cpp src/PidController.cpp)
target_link_libraries(run_tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(run_tests)