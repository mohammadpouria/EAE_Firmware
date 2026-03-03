#include "CANParser.hpp"

namespace eae::drivers {

    bool CANParser::parseFrame(const CANFrame& f) {
        std::lock_guard<std::mutex> lk(mtx_);

        // if the payload is empty, it's not a valid frame for our purposes
        if (f.dlc == 0) return false;

        switch (f.id) {
            case CAN_ID_TEMPERATURE:
                if (f.dlc >= 2) {
                    // Reconstruct 16-bit signed int (big-endian)
                    int16_t raw_temp = (f.payload[0] << 8) | f.payload[1]; 
                    data_.temperature_c = static_cast<float>(raw_temp) / 100.0f;
                    return true;
                }
                break;

            case CAN_ID_LEVEL:
                data_.level_ok = (f.payload[0] != 0);
                return true;

            case CAN_ID_PUMP_FEEDBACK:
                data_.pump_running = (f.payload[0] != 0);
                return true;

            case CAN_ID_SETPOINT_OVERRIDE:
                data_.setpoint_c = static_cast<double>(f.payload[0]);
                return true;

            default:
                break; // Unrecognized frame
        }
        
        return false;
    }

    ParsedData CANParser::read() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return data_;
    }

} // namespace eae::drivers