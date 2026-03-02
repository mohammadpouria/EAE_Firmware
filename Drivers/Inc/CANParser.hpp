#pragma once
#include "CANSocket.hpp"
#include "CANDefs.hpp"
#include "Config.hpp"
#include <mutex>

namespace eae::drivers {

    struct ParsedData {
        double temperature_c{25.0};
        bool level_ok{true};
        bool pump_running{false};
        double setpoint_c{eae::config::DEFAULT_SETPOINT_C};
    };

    class CANParser {
    public:
        CANParser() = default;

        // Parse a frame and update internal state if recognized
        bool parseFrame(const CANFrame& frame);

        // Thread-safe Asynchronous read of the latest data
        ParsedData latest() const;

    private:
        mutable std::mutex mtx_;
        ParsedData data_;
    };

}