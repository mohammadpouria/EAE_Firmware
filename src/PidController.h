#pragma once

class PidController {
private:
    float kp, ki, kd;
    float integral;
    float previous_error;
    float output_min, output_max;

public:
    PidController(float p, float i, float d, float min_out, float max_out);
    float calculate(float setpoint, float current_value, float dt);
    void reset();
};