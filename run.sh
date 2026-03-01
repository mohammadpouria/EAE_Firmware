#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

echo "=== Building EAE Firmware ==="
mkdir -p build
cd build

# Generate makefiles and compile
cmake ..
cmake --build .

echo ""
echo "=== Running Unit Tests ==="
./run_tests

echo ""
echo "=== Launching Firmware Simulation ==="
# Passing Command Line Arguments (Req 4)
# --target_temp: The setpoint for our PID controller
# --ignition: Simulate the key switch state
./cooling_sys --target_temp 45.0 --ignition 1