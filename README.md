# EAE Firmware Challenge: Active cooling loop for an inverter and DC-DC

This repository contains the firmware logic and simulation environment for the EAE Firmware Challenge. 

It implements a C++ control system for a mobile heavy-duty EV cooling loop, managing an EMP WP32 water pump and a SPAL VA97 brushless fan to maintain optimal operating temperatures for the Traction Inverter and DC-DC converter.

## 🎯 Challenge Requirements Fulfilled

This project was built to satisfy the specific architectural requirements of Section 7.1:

1. **Simulated CAN Bus (J1939):** Implements a mock Hardware Abstraction Layer (HAL) to demonstrate bare-metal byte-packing and hardware-level register simulation.
2. **PID Loop:** A continuous discrete PID controller manages the pump and fan's PWM duty cycles, featuring integral anti-windup clamping to prevent overshoot.
3. **State Machine:** A robust state machine (`INIT`, `IDLE`, `ACTIVE_COOLING`, `FAULT`) handles system transitions, safety cutoffs (low coolant), and thermal hysteresis to prevent actuator chatter.
4. **Command Line Arguments:** Setpoints and ignition states are passed dynamically at runtime.
5. **CMake Build System:** Fully managed CMake pipeline handling both the application and test executables.
6. **Linux/MSYS2 Shell Script:** A provided `run.sh` automates the build process, test execution, and simulation launch.
7. **GTest Unit Testing:** Automated unit tests verify the mathematical integrity of the PID controller.
8. **No Shipped Dependencies:** GTest is dynamically fetched and statically linked at configure-time via CMake's `FetchContent` module.

## 📂 Project Structure

```text
EAE_Firmware/
├── CMakeLists.txt          # Primary build configuration
├── run.sh                  # Build, test, and execution script
├── src/
│   ├── main.cpp            # Application entry & CLI parser
│   ├── StateMachine.h/cpp  # Core safety & state logic
│   ├── PidController.h/cpp # Mathematical control loop
│   └── CanBus.h/cpp        # Mock HAL & J1939 formatting
└── tests/
    └── test_pid.cpp        # Google Test suite