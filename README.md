# EAE Firmware - Thermal Control System

A C++17 embedded Linux firmware application designed to simulate and control a heavy-machinery cooling loop. This project demonstrates production-grade firmware architecture, including hardware abstraction, finite state machines, discrete PID control, and real-time SocketCAN networking.

## 🏗️ Architecture

The codebase strictly follows a modular, hardware-agnostic design pattern commonly used in bare-metal and RTOS environments, separated into two primary domains:

* **`Drivers/` (Hardware Abstraction Layer):** Handles all low-level Linux system calls and network interactions. 
  * `CANSocket`: An object-oriented wrapper around the raw Linux SocketCAN API.
  * `CANParser`: A thread-safe data decoder that translates raw J1939-style hex bytes into clean engineering units.
* **`Core/` (Application Logic):** 100% hardware-agnostic control logic, easily portable to an STM32 or FreeRTOS environment.
  * `StateMachine`: Manages system safety, pump priming timers, and fault handling (e.g., sudden coolant loss).
  * `PidController`: A discrete proportional-integral-derivative controller featuring integral anti-windup clamping for dynamic fan and pump speed control.

## ⚙️ Requirements

To build and simulate this firmware, you need a Linux environment (Ubuntu or WSL2) with the following tools installed:

* A C++17 compatible compiler (`g++` or `clang`)
* CMake (v3.14+)
* `can-utils` (for network simulation)
* `iproute2` (for `vcan0` setup)

To install the dependencies on Ubuntu/WSL:
```bash
sudo apt update
sudo apt install build-essential cmake can-utils