cmake_minimum_required(VERSION 3.14)
project(EAE_Firmware LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Output all compiled binaries to the build directory root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

enable_testing()

# --- Google Test Fetch ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Prevent GTest from overriding compiler flags
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Drivers Module ---
add_library(drivers STATIC
    Drivers/Src/CANParser.cpp
    Drivers/Src/CANSocket.cpp
)
target_include_directories(drivers PUBLIC Drivers/Inc Core/Inc)

# --- Application Core Module ---
add_library(appcore STATIC
    Core/Src/StateMachine.cpp
    Core/Src/PidController.cpp
)
target_include_directories(appcore PUBLIC Core/Inc Drivers/Inc)
# App logic depends on the hardware data formats
target_link_libraries(appcore PUBLIC drivers)

# --- Main Firmware ---
add_executable(eae_firmware Core/Src/main.cpp)
target_include_directories(eae_firmware PRIVATE Core/Inc)
target_link_libraries(eae_firmware PRIVATE appcore drivers)

# --- Unit Tests ---
# --- State Machine Tests ---
add_executable(test_statemachine tests/test_statemachine.cpp)
target_link_libraries(test_statemachine PRIVATE appcore gtest_main)
target_include_directories(test_statemachine PRIVATE Core/Inc Drivers/Inc)
add_test(NAME statemachine_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_statemachine)

# --- PID Tests ---
add_executable(test_pid tests/test_pid.cpp)
target_link_libraries(test_pid PRIVATE appcore gtest_main)
target_include_directories(test_pid PRIVATE Core/Inc)
add_test(NAME pid_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_pid)  

# --- CAN Parser Tests ---
add_executable(test_canparser tests/test_canparser.cpp)
target_link_libraries(test_canparser PRIVATE drivers gtest_main)
target_include_directories(test_canparser PRIVATE Drivers/Inc Core/Inc)
add_test(NAME canparser_test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_canparser)