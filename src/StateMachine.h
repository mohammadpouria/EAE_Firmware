#pragma once
#include "PidController.h"

enum class SystemState {
    INIT,
    IDLE,
    ACTIVE_COOLING,
    FAULT
};

struct SystemOutputs {
    float pumpSpeed;
    float fanSpeed;
};

class StateMachine {
private:
    SystemState currentState;
    PidController fanPid;
    PidController pumpPid;
    float targetTemp;

public:
    StateMachine(float setpoint);
    SystemOutputs update(bool ignition, bool levelOk, float currentTemp, float dt);
    SystemState getState() const { return currentState; }
};