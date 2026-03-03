#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <iomanip>
#include <string>

#include "Config.hpp"
#include "CANSocket.hpp"
#include "CANParser.hpp"
#include "StateMachine.hpp"

using namespace eae;

// Global flag to allow clean shutdown on SIGINT (Ctrl+C)
std::atomic<bool> keep_running{true};

void signal_handler(int signum) {
    keep_running = false;
}

int main(int argc, char* argv[]) {
    // Register signal handler for graceful exit
    std::signal(SIGINT, signal_handler);

    // Parse Command Line Arguments for Setpoint
    double setpoint = config::DEFAULT_SETPOINT_C;
    if (argc > 1) {
        try {
            setpoint = std::stod(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "[WARN] Invalid setpoint argument. Using default: " << setpoint << "°C\n";
        }
    }

    std::cout << "==========================================\n";
    std::cout << " EAE Firmware - Cooling Control Simulator \n";
    std::cout << "==========================================\n";
    std::cout << "[INFO] Target Setpoint: " << setpoint << "°C\n";

    // 3. Initialize Drivers (Hardware Abstraction)
    drivers::CANSocket can_socket("vcan0");
    if (!can_socket.open_socket()) {
        std::cerr << "[FATAL] Could not open CAN socket. Exiting.\n";
        return -1;
    }
    std::cout << "[INFO] Connected to vcan0\n";

    drivers::CANParser can_parser;
    
    // Inject the CLI setpoint into the parser using a mock CAN frame
    drivers::CANFrame override_frame;
    override_frame.id = drivers::CAN_ID_SETPOINT_OVERRIDE;
    override_frame.payload = { static_cast<uint8_t>(setpoint) };
    can_parser.parseFrame(override_frame);
    // can_socket.send_frame(override_frame); 
    
    // Initialize App Core
    app::StateMachine state_machine;
    drivers::ParsedData sensors;
    app::SystemOutputs outputs;

    // Main Control Loop Timing setup (200ms per Config.hpp)
    const auto loop_dt = std::chrono::milliseconds(static_cast<int>(config::CONTROL_DT_SEC * 1000));
    std::string log_line = "";

    while (keep_running) {
        auto loop_start_time = std::chrono::steady_clock::now();

        // --- Drain the CAN RX Queue ---
        // Read until the socket buffer is empty to ensure we have the absolute latest data
        drivers::CANFrame rx_frame;
        while (can_socket.recv_frame(rx_frame)) {
            can_parser.parseFrame(rx_frame);
            sensors = can_parser.read(); // Update the sensors struct with the latest data
        }

        // --- Execute Control Logic ---
        app::SystemOutputs outputs_tmp = state_machine.run_step(sensors, config::CONTROL_DT_SEC);

        // --- Transmit Actuator Commands ---
        // Pack the outputs into the CAN_ID_STATUS (0x300)
        if (outputs_tmp != outputs) {
            outputs = outputs_tmp;
            drivers::CANFrame tx_frame;
            tx_frame.id = drivers::CAN_ID_STATUS;
            tx_frame.dlc = 3; // We will send 3 bytes: pump speed, fan speed, derate flag
            tx_frame.payload[0] = static_cast<uint8_t>(outputs.pumpSpeedPct);
            tx_frame.payload[1] = static_cast<uint8_t>(outputs.fanSpeedPct);
            tx_frame.payload[2] = outputs.derateRequest ? 1 : 0;

            can_socket.send_frame(tx_frame); // Send the control commands back on the CAN bus

            // --- Console Output (Dashboard style) ---
            std::cout << "[CTRL] State=" << std::left << std::setw(15) << state_machine.getStateName()
                    << " T=" << std::setw(4) << sensors.temperature_c << "°C  "
                    << " Fan=" << std::setw(3) << static_cast<int>(outputs.fanSpeedPct) << "%  "
                    << " Pump=" << std::setw(3) << static_cast<int>(outputs.pumpSpeedPct) << "%  "
                    << " Derate=" << (outputs.derateRequest ? "Y" : "N") 
                    << "\n"; 
        }
        

        // --- Precision Sleep ---
        // Calculate how long the logic took to run, and only sleep for the remainder of the 200ms
        auto loop_end_time = std::chrono::steady_clock::now();
        auto elapsed = loop_end_time - loop_start_time;
        if (elapsed < loop_dt) {
            std::this_thread::sleep_for(loop_dt - elapsed);
        }
    }

    // Cleanup
    std::cout << "\n[INFO] Shutting down gracefully...\n";
    can_socket.close_socket();
    return 0;
}