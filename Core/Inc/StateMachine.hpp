#pragma once
#include "PidController.hpp"
#include "CANParser.hpp"
#include <string>

namespace eae::app {

    enum class SystemState {
        OFF,
        INIT,
        IDLE,
        ACTIVE_COOLING,
        BOOST_COOLING,
        FAULT
    };

    struct SystemOutputs {
        double pumpSpeedPct{0.0};
        double fanSpeedPct{0.0};
        bool derateRequest{false};

        // Define the != operator
        bool operator!=(const SystemOutputs& other) const {
            // Define our tolerance (e.g., 0.001%)
            const double EPSILON = 0.001; 

            return (std::abs(pumpSpeedPct - other.pumpSpeedPct) > EPSILON) ||
                (std::abs(fanSpeedPct - other.fanSpeedPct) > EPSILON) ||
                (derateRequest != other.derateRequest);
        }

        bool operator==(const SystemOutputs& other) const {
            return !(*this != other);
        }
    };

    /**
     * \brief Manages the thermal control logic and system states.
     *
     * Evaluates incoming sensor data, manages safety fault conditions (e.g., low coolant), 
     * enforces minimum state durations, and computes the necessary PID outputs for the hardware.
     */
    class StateMachine {
    private:
        SystemState currentState_{SystemState::INIT};
        PidController fanPid;
        PidController pumpPid;
        double state_timer_ms_{0.0};

        void transitionTo(SystemState newState);

    public:
        // Initialize with default target temperature
        StateMachine();
        
        // The unified orchestrator function
        SystemOutputs run_step(const drivers::ParsedData& sensors, double dt_sec);
        
        SystemState getState() const { return currentState_; }
        std::string getStateName() const;
    };

}