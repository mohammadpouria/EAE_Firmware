#include "StateMachine.hpp"
#include "Config.hpp"
#include <iostream>

namespace eae::app {

    StateMachine::StateMachine() 
        : currentState_(SystemState::INIT),
          state_timer_ms_(0.0),
          fanPid(5.0, 0.5, 0.1, 0.0, 100.0),   // Kp, Ki, Kd, Min, Max
          pumpPid(10.0, 0.0, 0.0, 20.0, 100.0) // P-only for pump. Minimum 20% to keep flow alive.
    {}

    void StateMachine::transitionTo(SystemState newState) {
        if (currentState_ != newState) {
            currentState_ = newState;
            state_timer_ms_ = 0.0; // Reset timer on state entry
        }
    }

    SystemOutputs StateMachine::run_step(const drivers::ParsedData& sensors, double dt_sec) {
        SystemOutputs outputs;
        state_timer_ms_ += (dt_sec * 1000.0); // Update state timer in milliseconds

        // Fault Check (Overrides everything)
        if (!sensors.level_ok) {
            if (currentState_ != SystemState::FAULT) {
                std::cout << ">>> TRANSITION: FAULT (Low Coolant) <<<\n";
                transitionTo(SystemState::FAULT);
            }
        } 

        // Logic Evaluation Based on Current State
        switch (currentState_) {
            case SystemState::INIT:
                outputs.pumpSpeedPct = 100.0;
                outputs.fanSpeedPct = 0.0;
                outputs.derateRequest = true;
                fanPid.reset();
                pumpPid.reset();
                
                // Assuming system is always "ignited" for this simulation if no ignition flag exists
                // Only run the priming sequence if we have coolant
                if (sensors.level_ok && state_timer_ms_ >= eae::config::PUMP_PRIME_MS) {
                    std::cout << ">>> TRANSITION: IDLE <<<\n";
                    transitionTo(SystemState::IDLE);
                }
                break;

            case SystemState::IDLE:
                outputs.pumpSpeedPct = 20.0; // Minimum flow for sensor reading
                outputs.fanSpeedPct = 0.0;
                outputs.derateRequest = false;

                if (sensors.temperature_c >= config::CRITICAL_TEMP_C) {
                    std::cout << ">>> TRANSITION: BOOST COOLING <<<\n";
                    transitionTo(SystemState::BOOST_COOLING);
                }
                else if (sensors.temperature_c >= sensors.setpoint_c) {
                    std::cout << ">>> TRANSITION: ACTIVE COOLING <<<\n";
                    transitionTo(SystemState::ACTIVE_COOLING);
                }
                break;

            case SystemState::ACTIVE_COOLING:
                // Both Pump and Fan are dynamically controlled by PID
                outputs.pumpSpeedPct = pumpPid.calculate(sensors.setpoint_c, sensors.temperature_c, dt_sec);
                outputs.fanSpeedPct = fanPid.calculate(sensors.setpoint_c, sensors.temperature_c, dt_sec);
                outputs.derateRequest = false;
                
                if (sensors.temperature_c >= config::CRITICAL_TEMP_C) {
                    std::cout << ">>> TRANSITION: BOOST COOLING <<<\n";
                    transitionTo(SystemState::BOOST_COOLING);
                }
                // Hysteresis: wait until it's cooler than target to go back to IDLE
                else if (sensors.temperature_c < (sensors.setpoint_c - config::HYSTERESIS_C)) {
                    std::cout << ">>> TRANSITION: IDLE <<<\n";
                    transitionTo(SystemState::IDLE);
                }
                break;

            case SystemState::BOOST_COOLING:
                outputs.pumpSpeedPct = 100.0;
                outputs.fanSpeedPct = 100.0;
                outputs.derateRequest = true; // Tell inverter to cut power

                if (state_timer_ms_ >= config::BOOST_DURATION_MS) {
                    if (sensors.temperature_c < (config::CRITICAL_TEMP_C - config::HYSTERESIS_C)) {
                        std::cout << ">>> TRANSITION: ACTIVE COOLING <<<\n";
                        transitionTo(SystemState::ACTIVE_COOLING);
                    }
                }
                break;

            case SystemState::FAULT:
                outputs.pumpSpeedPct = 0.0;   // Stop pump to prevent dry run
                outputs.fanSpeedPct = 100.0;  // Max fan to reject residual heat
                outputs.derateRequest = true; 
                
                if (sensors.level_ok) {
                    std::cout << ">>> TRANSITION: INIT (Fault Cleared) <<<\n";
                    transitionTo(SystemState::INIT);
                }
                break;
                
            case SystemState::OFF:
                // Included for enum completeness
                break;
        }

        return outputs;
    }

    std::string StateMachine::getStateName() const {
        switch (currentState_) {
            case SystemState::OFF: return "OFF";
            case SystemState::INIT: return "INIT";
            case SystemState::IDLE: return "IDLE";
            case SystemState::ACTIVE_COOLING: return "ACTIVE_COOLING";
            case SystemState::BOOST_COOLING: return "BOOST_COOLING";
            case SystemState::FAULT: return "FAULT";
            default: return "UNKNOWN";
        }
    }

}