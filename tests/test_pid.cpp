#include <gtest/gtest.h>
#include "PidController.hpp"

using namespace eae::app;

// Test that output is 0 when we are exactly at the target temperature
TEST(PidTest, ZeroErrorYieldsZeroOutput) {
    PidController pid(2.0, 0.1, 0.0, 0.0, 100.0);
    double output = pid.calculate(45.0, 45.0, 1.0);
    EXPECT_DOUBLE_EQ(output, 0.0);
}

// Test that proportional logic pushes output up when it's too hot
TEST(PidTest, PositiveErrorYieldsPositiveOutput) {
    PidController pid(2.0, 0.0, 0.0, 0.0, 100.0); // P-only controller
    double output = pid.calculate(45.0, 55.0, 1.0); // 10 degrees too hot
    EXPECT_DOUBLE_EQ(output, 20.0); // Error(10) * Kp(2.0) = 20.0
}

// Test that the output properly clamps at the maximum allowed value
TEST(PidTest, OutputClampsAtMax) {
    PidController pid(10.0, 0.0, 0.0, 0.0, 100.0); 
    double output = pid.calculate(45.0, 80.0, 1.0); // 35 degrees too hot
    // Error(35) * Kp(10) = 350. Should clamp to 100.
    EXPECT_DOUBLE_EQ(output, 100.0); 
}