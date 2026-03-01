#include <gtest/gtest.h>
#include "../src/PidController.h"

// Test that output is 0 when we are exactly at the target temperature
TEST(PidTest, ZeroErrorYieldsZeroOutput) {
    PidController pid(2.0f, 0.1f, 0.0f, 0.0f, 100.0f);
    float output = pid.calculate(45.0f, 45.0f, 1.0f);
    EXPECT_FLOAT_EQ(output, 0.0f);
}

// Test that proportional logic pushes output up when it's too hot
TEST(PidTest, PositiveErrorYieldsPositiveOutput) {
    PidController pid(2.0f, 0.0f, 0.0f, 0.0f, 100.0f); // P-only controller
    float output = pid.calculate(45.0f, 55.0f, 1.0f); // 10 degrees too hot
    EXPECT_FLOAT_EQ(output, 20.0f); // Error(10) * Kp(2.0) = 20.0
}

// Test that the output properly clamps at the maximum allowed value
TEST(PidTest, OutputClampsAtMax) {
    PidController pid(10.0f, 0.0f, 0.0f, 0.0f, 100.0f); 
    float output = pid.calculate(45.0f, 80.0f, 1.0f); // 35 degrees too hot
    // Error(35) * Kp(10) = 350. Should clamp to 100.
    EXPECT_FLOAT_EQ(output, 100.0f); 
}