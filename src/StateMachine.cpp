#include "StateMachine.h"
#include <iostream>

StateMachine::StateMachine(float setpoint) 
    : currentState(SystemState::INIT), 
      fanPid(5.0f, 0.5f, 0.1f, 0.0f, 100.0f), // Kp, Ki, Kd, Min, Max
      pumpPid(10.0f, 0.0f, 0.0f, 0.0f, 100.0f), // P-only for pump
      targetTemp(setpoint) {}

SystemOutputs StateMachine::update(bool ignition, bool levelOk, float currentTemp, float dt) {
    SystemOutputs outputs = {0.0f, 0.0f}; // default outputs

    // Check Safety Critical Faults first
    if (!levelOk) {
        if (currentState != SystemState::FAULT) {
            std::cout << ">>> TRANSITION: FAULT (Low Coolant) <<<\n";
            currentState = SystemState::FAULT;
        }
    } else if (!ignition && currentState != SystemState::FAULT) {
        currentState = SystemState::INIT;
    }

    // State Machine Execution
    switch (currentState) {
        case SystemState::INIT:
            outputs.pumpSpeed = 0.0f;
            outputs.fanSpeed = 0.0f;
            fanPid.reset();
            pumpPid.reset();
            if (ignition && levelOk) {
                std::cout << ">>> TRANSITION: IDLE <<<\n";
                currentState = SystemState::IDLE;
            }
            break;

        case SystemState::IDLE:
            outputs.pumpSpeed = 20.0f; // Minimum flow for sensor reading
            outputs.fanSpeed = 0.0f;
            if (currentTemp >= targetTemp) {
                std::cout << ">>> TRANSITION: ACTIVE COOLING <<<\n";
                currentState = SystemState::ACTIVE_COOLING;
            }
            break;

        case SystemState::ACTIVE_COOLING:
            // High flow pump when cooling
            outputs.pumpSpeed = pumpPid.calculate(targetTemp, currentTemp, dt);
            // PID handles the fan speed
            outputs.fanSpeed = fanPid.calculate(targetTemp, currentTemp, dt);
            
            // Hysteresis: wait until it's 2 degrees cooler than target to go back to IDLE
            if (currentTemp < (targetTemp - 2.0f)) {
                std::cout << ">>> TRANSITION: IDLE <<<\n";
                currentState = SystemState::IDLE;
                fanPid.reset();
            }
            break;

        case SystemState::FAULT:
            outputs.pumpSpeed = 0.0f; // Stop pump to prevent dry run
            outputs.fanSpeed = 100.0f; // Max fan to reject residual heat
            if (levelOk && ignition) {
                std::cout << ">>> TRANSITION: INIT (Fault Cleared) <<<\n";
                currentState = SystemState::INIT;
            }
            break;
    }

    return outputs;
}