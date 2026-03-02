#pragma once

namespace eae::config {
    // Control Loop Parameters
    constexpr double CONTROL_DT_SEC = 0.2; // 200 ms loop execution time
    constexpr double DEFAULT_SETPOINT_C = 65.0;
    constexpr double HYSTERESIS_C = 2.0;
    
    // Thermal Safety Thresholds
    constexpr double DERATE_TEMP_C = 75.0;
    constexpr double CRITICAL_TEMP_C = 95.0;
    
    // System Timeouts (for fault detection)
    constexpr int SENSOR_TIMEOUT_MS = 400;
    constexpr int PUMP_FEEDBACK_TIMEOUT_MS = 1000;
    constexpr int WATCHDOG_TIMEOUT_MS = 500;
    
    // Operating Durations
    constexpr int PUMP_PRIME_MS = 3000;
    constexpr int BOOST_DURATION_MS = 30000;
}