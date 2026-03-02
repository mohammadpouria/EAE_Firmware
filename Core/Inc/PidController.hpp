#pragma once

namespace eae::app {

    /**
     * \brief Proportional-Integral-Derivative (PID) controller implementation.
     *
     * This class provides a standard discrete-time PID control algorithm with 
     * built-in anti-windup protection (via output clamping). It is used to smoothly 
     * regulate hardware actuators like cooling fans and pumps based on thermal error over time.
     */
    class PidController {
    private:
        float kp_, ki_, kd_;
        float integral_{0.0};
        float previous_error_{0.0};
        float output_min_, output_max_;

    public:
        PidController(float p, float i, float d, float min_out, float max_out);
        float calculate(float setpoint, float current_value, float dt);
        void reset();
    };
}