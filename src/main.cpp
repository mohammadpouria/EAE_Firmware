#include <iostream>
#include <string>
#include <vector>
#include "StateMachine.h"
#include "CanBus.h"

int main(int argc, char* argv[]) {
    // Default arguments
    float target_temp = 45.0f;
    bool ignition = true;

    // Parse command line arguments (Requirement 4)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--target_temp" && i + 1 < argc) {
            target_temp = std::stof(argv[++i]);
        } else if (arg == "--ignition" && i + 1 < argc) {
            ignition = std::stoi(argv[++i]) != 0;
        }
    }

    std::cout << "=== EAE Firmware Booting ===\n";
    std::cout << "Setpoint: " << target_temp << " C | Ignition: " << (ignition ? "ON" : "OFF") << "\n\n";

    // Initialize State Machine and CAN Bus
    StateMachine sm(target_temp);
    CanBus can;
    can.init();

    // Simulated environment data (Time step: 1.0 second)
    // Simulating: Cold -> Warming -> Target -> Overheating -> Coolant Leak
    std::vector<float> simulated_temps = {30.0, 35.0, 42.0, 45.1, 46.5, 48.0, 44.0, 42.0};
    bool coolant_level_ok = true;

    for (size_t i = 0; i < simulated_temps.size(); ++i) {
        std::cout << "\n[T=" << i << "s] Sensor Temp: " << simulated_temps[i] << " C\n";
        
        // Simulate a broken hose at t=6
        if (i == 6) coolant_level_ok = false; 

        SystemOutputs out = sm.update(ignition, coolant_level_ok, simulated_temps[i], 1.0f);
        can.sendCoolingCommand(out.pumpSpeed, out.fanSpeed);
    }

    return 0;
}