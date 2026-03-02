#include "CANParser.hpp"

namespace eae::drivers {

    bool CANParser::parseFrame(const CANFrame& f) {
        std::lock_guard<std::mutex> lk(mtx_);

        if (f.data.empty()) return false;

        switch (f.id) {
            case CAN_ID_TEMPERATURE:
                if (f.data.size() >= 2) {
                    // Reconstruct 16-bit signed int (big-endian)
                    int16_t raw_temp = (f.data[0] << 8) | f.data[1]; 
                    data_.temperature_c = static_cast<float>(raw_temp) / 100.0f;
                    return true;
                }
                break;

            case CAN_ID_LEVEL:
                data_.level_ok = (f.data[0] != 0);
                return true;

            case CAN_ID_PUMP_FEEDBACK:
                data_.pump_running = (f.data[0] != 0);
                return true;

            case CAN_ID_SETPOINT_OVERRIDE:
                data_.setpoint_c = static_cast<double>(f.data[0]);
                return true;

            default:
                break; // Unrecognized frame
        }
        
        return false;
    }

    ParsedData CANParser::latest() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return data_;
    }

} // namespace eae::drivers