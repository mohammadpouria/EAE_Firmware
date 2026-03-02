#include "PidController.hpp"
#include <algorithm>

namespace eae::app {

    PidController::PidController(float p, float i, float d, float min_out, float max_out) 
        : kp_(p), ki_(i), kd_(d), integral_(0.0f), previous_error_(0.0f), 
          output_min_(min_out), output_max_(max_out) {}

    float PidController::calculate(float setpoint, float current_value, float dt) {
        if (dt <= 0.0f) return 0.0f;

        float error = current_value - setpoint; // Positive error means it's too hot

        // Proportional
        float p_out = kp_ * error;

        // Integral (with basic anti-windup clamping)
        integral_ += error * dt;
        float i_out = ki_ * integral_;
        i_out = std::clamp(i_out, output_min_, output_max_);

        // Derivative
        float derivative = (error - previous_error_) / dt;
        float d_out = kd_ * derivative;

        // Total Output
        float output = p_out + i_out + d_out;

        // Save error for next cycle
        previous_error_ = error;

        // Clamp final output to allowed range (e.g., 0% to 100% PWM)
        return std::clamp(output, output_min_, output_max_);
    }

    void PidController::reset() {
        integral_ = 0.0f;
        previous_error_ = 0.0f;
    }
}