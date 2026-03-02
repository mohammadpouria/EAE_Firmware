#include <gtest/gtest.h>
#include "StateMachine.hpp"
#include "Config.hpp"

using namespace eae::app;
using namespace eae::drivers;

class StateMachineTest : public ::testing::Test {
protected:
    StateMachine sm;
    ParsedData mock_sensors;
    const double DT = 0.2; // 200ms per step
    
    // Helper to simulate time passing by calling run_step multiple times
    void advance_time_ms(int milliseconds) {
        int steps = milliseconds / (DT * 1000);
        for (int i = 0; i < steps; ++i) {
            sm.run_step(mock_sensors, DT);
        }
    }

};

// Test 1: Critical fault triggers immediately if coolant level drops
TEST_F(StateMachineTest, ImmediateFaultOnLowCoolant) {
    mock_sensors.level_ok = false;
    SystemOutputs outputs = sm.run_step(mock_sensors, DT);
    
    EXPECT_EQ(sm.getState(), SystemState::FAULT);
    EXPECT_DOUBLE_EQ(outputs.pumpSpeedPct, 0.0);   // Pump MUST stop
    EXPECT_DOUBLE_EQ(outputs.fanSpeedPct, 100.0);  // Fan MUST max out
    EXPECT_TRUE(outputs.derateRequest);            // MUST cut power
}

// Test 2: Startup Sequence (INIT -> Prime Timer -> IDLE)
TEST_F(StateMachineTest, StartupPrimingSequence) {
    mock_sensors.level_ok = true;
    mock_sensors.temperature_c = 25.0; // Cool system
    
    // First step should enter INIT and start the pump
    SystemOutputs outputs = sm.run_step(mock_sensors, DT);
    EXPECT_EQ(sm.getState(), SystemState::INIT);
    EXPECT_DOUBLE_EQ(outputs.pumpSpeedPct, 100.0); 
    
    // Advance time just *before* the prime duration finishes
    advance_time_ms(eae::config::PUMP_PRIME_MS - 400);
    EXPECT_EQ(sm.getState(), SystemState::INIT); // Should still be priming
    
    // Advance one more step to cross the threshold
    sm.run_step(mock_sensors, DT);
    EXPECT_EQ(sm.getState(), SystemState::IDLE); // Should now transition to IDLE
}

// Test 3: Boost Cooling locks in for the minimum duration
TEST_F(StateMachineTest, BoostCoolingEnforcesTimer) {
    mock_sensors.level_ok = true;
    mock_sensors.setpoint_c = 70.0;
    
    // Force system into IDLE first
    advance_time_ms(eae::config::PUMP_PRIME_MS + 200); 
    
    // Spike temperature past critical
    mock_sensors.temperature_c = 96.0; 
    sm.run_step(mock_sensors, DT);
    EXPECT_EQ(sm.getState(), SystemState::BOOST_COOLING);
    
    // Drop the temperature immediately back to safe levels (60°C)
    mock_sensors.temperature_c = 75.0;
    sm.run_step(mock_sensors, DT);
    
    // It should STILL be in BOOST_COOLING because the timer hasn't expired!
    EXPECT_EQ(sm.getState(), SystemState::BOOST_COOLING);
    
    // Advance past the 30-second boost duration
    advance_time_ms(eae::config::BOOST_DURATION_MS);
    
    // NOW it should transition back down
    EXPECT_EQ(sm.getState(), SystemState::ACTIVE_COOLING);
}