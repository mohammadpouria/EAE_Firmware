#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

echo "=== Setting up vcan0 Network ==="
sudo modprobe vcan || true

if ! ip link show vcan0 > /dev/null 2>&1; then
    sudo ip link add dev vcan0 type vcan
    sudo ip link set up vcan0
    echo "[INFO] vcan0 created and brought up."
else
    echo "[INFO] vcan0 already exists."
fi

echo ""
echo "=== Building EAE Firmware ==="
mkdir -p build
cd build
cmake ..
cmake --build . -j

echo ""
echo "=== Running Unit Tests ==="
ctest --output-on-failure

echo ""
echo "=== Launching Firmware & Simulating CAN Traffic ==="
# Launch firmware in the background (&) and save its Process ID
killall eae_firmware 2>/dev/null || true
./eae_firmware 70 &
FIRMWARE_PID=$!

sleep 1

echo -e "\n[SIM] Sending Level OK (0x101) & Pump Feedback OK (0x102)..."
cansend vcan0 101#01
cansend vcan0 102#01
sleep 4 

echo -e "\n[SIM] Injecting Heat: 60.5°C (ID: 0x100 -> Hex: 0x17AC)..."
cansend vcan0 100#17AC
sleep 3 

echo -e "\n[SIM] Injecting Heat: 72.0°C (ID: 0x100 -> Hex: 0x1C20)..."
cansend vcan0 100#1C20
sleep 3 

echo -e "\n[SIM] Injecting Heat: 85.5°C (ID: 0x100 -> Hex: 0x2166)..."
cansend vcan0 100#2166
sleep 3 

echo -e "\n[SIM] Injecting Critical Heat: 96.0°C (ID: 0x100 -> Hex: 0x2580)..."
cansend vcan0 100#2580
sleep 3 

echo -e "\n[SIM] Simulating Sudden Coolant Loss (ID: 0x101 -> 0x00)..."
cansend vcan0 101#00
sleep 2 

echo -e "\n=== Simulation Complete ==="
kill -SIGINT $FIRMWARE_PID
wait $FIRMWARE_PID 2>/dev/null || true
echo "Firmware stopped safely."