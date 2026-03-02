#pragma once
#include <cstdint>

/**
 * \brief Data structures and constants for CAN bus communication.
 *
 * This file contains the standardized data structures used to pass decoded 
 * sensor information from the low-level CAN drivers up to the application-level 
 * control logic, cleanly decoupling the hardware from the State Machine.
 */
namespace eae::drivers {
    // CAN IDs for cooling loop instrumentation
    enum CAN_ID : uint32_t {
        CAN_ID_TEMPERATURE       = 0x100,
        CAN_ID_LEVEL             = 0x101,
        CAN_ID_PUMP_FEEDBACK     = 0x102,
        CAN_ID_SETPOINT_OVERRIDE = 0x200,
        CAN_ID_STATUS            = 0x300 // Used by the firmware to broadcast its state
    };
}