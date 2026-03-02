#include <gtest/gtest.h>
#include "CANParser.hpp"
#include "CANDefs.hpp"
#include "Config.hpp"

using namespace eae::drivers;

// Test 1: Verify the parser initializes with safe default values
TEST(CANParserTest, SafeDefaultValues) {
    CANParser parser;
    ParsedData data = parser.latest();
    
    EXPECT_DOUBLE_EQ(data.temperature_c, 25.0);
    EXPECT_TRUE(data.level_ok);
    EXPECT_FALSE(data.pump_running);
    EXPECT_DOUBLE_EQ(data.setpoint_c, eae::config::DEFAULT_SETPOINT_C);
}

// Test 2: Verify Temperature Parsing (Positive value, 16-bit Big-Endian)
TEST(CANParserTest, ParsesPositiveTemperature) {
    CANParser parser;
    CANFrame frame;
    frame.id = CAN_ID_TEMPERATURE;
    
    // Let's simulate 85.50°C (which is 8550 centi-degrees)
    // 8550 in Hex is 0x2166. Big Endian means 0x21 comes first.
    frame.data = {0x21, 0x66};
    
    EXPECT_TRUE(parser.parseFrame(frame));
    EXPECT_DOUBLE_EQ(parser.latest().temperature_c, 85.50);
}

// Test 3: Verify Temperature Parsing (Negative value, Two's Complement)
TEST(CANParserTest, ParsesNegativeTemperature) {
    CANParser parser;
    CANFrame frame;
    frame.id = CAN_ID_TEMPERATURE;
    
    // Simulate -10.00°C (-1000 centi-degrees)
    // -1000 in 16-bit Two's Complement Hex is 0xFC18
    frame.data = {0xFC, 0x18};
    
    parser.parseFrame(frame);
    EXPECT_DOUBLE_EQ(parser.latest().temperature_c, -10.00);
}

// Test 4: Verify Level Sensor Fault Detection
TEST(CANParserTest, ParsesLevelFault) {
    CANParser parser;
    CANFrame frame;
    frame.id = CAN_ID_LEVEL;
    frame.data = {0x00}; // 0 = Fault, 1 = OK
    
    parser.parseFrame(frame);
    EXPECT_FALSE(parser.latest().level_ok);
}