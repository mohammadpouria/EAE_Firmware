#include "PidController.h"
#include <algorithm>

PidController::PidController(float p, float i, float d, float min_out, float max_out) 
    : kp(p), ki(i), kd(d), integral(0.0f), previous_error(0.0f), 
      output_min(min_out), output_max(max_out) {}

float PidController::calculate(float setpoint, float current_value, float dt) {
    if (dt <= 0.0f) return 0.0f;

    float error = current_value - setpoint; // Positive error means it's too hot

    // Proportional
    float p_out = kp * error;

    // Integral (with basic anti-windup clamping)
    integral += error * dt;
    float i_out = ki * integral;
    i_out = std::clamp(i_out, output_min, output_max);

    // Derivative
    float derivative = (error - previous_error) / dt;
    float d_out = kd * derivative;

    // Total Output
    float output = p_out + i_out + d_out;

    // Save error for next cycle
    previous_error = error;

    // Clamp final output to allowed range (e.g., 0% to 100% PWM)
    return std::clamp(output, output_min, output_max);
}

void PidController::reset() {
    integral = 0.0f;
    previous_error = 0.0f;
}