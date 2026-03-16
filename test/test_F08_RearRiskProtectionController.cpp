/**
 * @file    test_F08_RearRiskProtectionController.cpp
 * @brief   Unit tests for F-08: Rear Risk Protection Controller Module
 *
 * @details Tests the protection logic mapping from risk level to Child Lock
 *          action and Warning triggers. Requirements:
 *            - Auto-lock ON when Risk HIGH.
 *            - Maintain ON when Risk HIGH and already ON.
 *            - Safe Fallback when sensor faulty.
 *            - Do nothing when ignition OFF.
 *            - Handle null pointers safely.
 *          Achieves high Statement, Branch and MC/DC coverage.
 *
 * @version 1.2.0
 * @date    2026-03-11 01:42 (KST)
 * @author  AI Model (v1.0.0): Gemini 3.5 Pro — Initial implementation
 * @author  AI Model (v1.1.0~1.2.0): Claude Sonnet 4.6 (Thinking) — Review & fix
 * @copyright Synetics 20 CopyrightⓒSynetics_
 * @note    v1.2.0: Added EXPECT_TRUE(eventLog) assertions for risk-HIGH and
 *          sensor-fault branches per SDD Table F-08 Output spec.
 * @note    v1.1.0: Updated SensorFault test to expect warningSound=true
 *          per corrected SAFE_LOCKED HMI alert policy.
 */

#include <gtest/gtest.h>
extern "C" {
#include "F08_RearRiskProtectionController.h"
#include "childlock_types.h"
}

/**
 * @brief Test fixture for F-08 using Google Test
 */
class F08RearRiskProtectionTest : public ::testing::Test {
protected:
    RearRiskProtectionInput_t input;
    RearRiskProtectionOutput_t output;

    void SetUp() override {
        // Safe, default input condition
        input.riskHigh = false;
        input.riskValid = true;
        input.currentCLState = CL_STATE_OFF;
        input.ignitionState = IGN_STATE_ON;

        // Reset output
        output.targetCLState = CL_STATE_OFF;
        output.warningMsgId = WARNING_MSG_NONE;
        output.warningSound = false;
        output.clChanged = false;
    }

    void TearDown() override {
        // Clean up common resources
    }
};

/* =========================================================================
 * 1. Positive Risk Threat Scenarios
 * ========================================================================= */

TEST_F(F08RearRiskProtectionTest, HighRisk_CLWasOff_AutoActivate) {
    input.riskHigh = true;
    input.currentCLState = CL_STATE_OFF;

    F08_RearRiskProtectionController_Run(&input, &output);

    EXPECT_EQ(output.targetCLState, CL_STATE_ON);
    EXPECT_TRUE(output.clChanged);
    EXPECT_EQ(output.warningMsgId, WARNING_MSG_REAR_RISK_HIGH);
    EXPECT_TRUE(output.warningSound);
    EXPECT_TRUE(output.eventLog);  /* SD-5: "자동 보호 이벤트 로그 기록" must be set */
}

TEST_F(F08RearRiskProtectionTest, HighRisk_CLWasOn_MaintainLock) {
    input.riskHigh = true;
    input.currentCLState = CL_STATE_ON;

    F08_RearRiskProtectionController_Run(&input, &output);

    EXPECT_EQ(output.targetCLState, CL_STATE_ON);
    EXPECT_FALSE(output.clChanged);
    EXPECT_EQ(output.warningMsgId, WARNING_MSG_CL_ALREADY_ON);
    EXPECT_TRUE(output.warningSound);
    EXPECT_TRUE(output.eventLog);  /* risk-HIGH always logs */
}

TEST_F(F08RearRiskProtectionTest, NoRisk_CLOff_RemainOff) {
    input.riskHigh = false;
    input.currentCLState = CL_STATE_OFF;

    F08_RearRiskProtectionController_Run(&input, &output);

    EXPECT_EQ(output.targetCLState, CL_STATE_OFF);
    EXPECT_FALSE(output.clChanged);
    EXPECT_EQ(output.warningMsgId, WARNING_MSG_NONE);
    EXPECT_FALSE(output.warningSound);
    EXPECT_FALSE(output.eventLog); /* No risk, no log */
}

/* =========================================================================
 * 2. Fault Injection & Safe State Scenarios
 * ========================================================================= */

TEST_F(F08RearRiskProtectionTest, SensorFault_InvalidRisk_ForceSafeState) {
    input.riskValid = false;
    input.currentCLState = CL_STATE_OFF;

    F08_RearRiskProtectionController_Run(&input, &output);

    EXPECT_EQ(output.targetCLState, CL_STATE_ON);
    EXPECT_TRUE(output.clChanged);
    EXPECT_EQ(output.warningMsgId, WARNING_MSG_SENSOR_FAULT);
    /* v1.1.0 Fix: warningSound MUST be true on fault (SAFE_LOCKED policy) */
    EXPECT_TRUE(output.warningSound);
    EXPECT_TRUE(output.eventLog);  /* FG-02: "로그/DTC 저장" must be set on fault */
}

TEST_F(F08RearRiskProtectionTest, SensorFault_CLWasAlreadyOn) {
    input.riskValid = false;
    input.currentCLState = CL_STATE_ON;

    F08_RearRiskProtectionController_Run(&input, &output);

    // Maintain ON
    EXPECT_EQ(output.targetCLState, CL_STATE_ON);
    EXPECT_FALSE(output.clChanged);
    EXPECT_EQ(output.warningMsgId, WARNING_MSG_SENSOR_FAULT);
}

/* =========================================================================
 * 3. Boundary & System State Scenarios
 * ========================================================================= */

TEST_F(F08RearRiskProtectionTest, SystemOff_IgnitionOff_DoNothing) {
    input.ignitionState = IGN_STATE_OFF;
    input.riskHigh = true; // Would normally trigger
    input.currentCLState = CL_STATE_OFF;

    F08_RearRiskProtectionController_Run(&input, &output);

    EXPECT_EQ(output.targetCLState, CL_STATE_OFF); // output stays neutral
    EXPECT_FALSE(output.clChanged);
    EXPECT_EQ(output.warningMsgId, WARNING_MSG_NONE);
}

TEST_F(F08RearRiskProtectionTest, Robustness_NullPointers) {
    // Missing input should not crash
    F08_RearRiskProtectionController_Run(NULL, &output);
    
    // Missing output should not crash
    F08_RearRiskProtectionController_Run(&input, NULL);
    
    // Both missing should not crash
    F08_RearRiskProtectionController_Run(NULL, NULL);

    SUCCEED();
}

/* =========================================================================
 * 4. Additional branch coverage
 * ========================================================================= */

TEST_F(F08RearRiskProtectionTest, NoRisk_CLOn_MaintainState_NoWarning)
{
    input.riskHigh = false;
    input.riskValid = true;
    input.currentCLState = CL_STATE_ON;

    F08_RearRiskProtectionController_Run(&input, &output);

    EXPECT_EQ(output.targetCLState, CL_STATE_ON); /* maintains current state */
    EXPECT_FALSE(output.clChanged);
    EXPECT_EQ(output.warningMsgId, WARNING_MSG_NONE);
    EXPECT_FALSE(output.warningSound);
    EXPECT_FALSE(output.eventLog);
}

TEST_F(F08RearRiskProtectionTest, IgnitionOff_HighRisk_NoAction)
{
    input.ignitionState = IGN_STATE_OFF;
    input.riskHigh = true;
    input.riskValid = true;
    input.currentCLState = CL_STATE_OFF;

    F08_RearRiskProtectionController_Run(&input, &output);

    /* IGN OFF: feature inactive, output stays neutral */
    EXPECT_EQ(output.targetCLState, CL_STATE_OFF);
    EXPECT_FALSE(output.clChanged);
    EXPECT_EQ(output.warningMsgId, WARNING_MSG_NONE);
    EXPECT_FALSE(output.eventLog);
}

TEST_F(F08RearRiskProtectionTest, IgnitionOff_SensorFault_NoAction)
{
    input.ignitionState = IGN_STATE_OFF;
    input.riskValid = false;
    input.currentCLState = CL_STATE_OFF;

    F08_RearRiskProtectionController_Run(&input, &output);

    /* IGN OFF: feature inactive even for sensor fault */
    EXPECT_EQ(output.targetCLState, CL_STATE_OFF);
    EXPECT_FALSE(output.clChanged);
    EXPECT_EQ(output.warningMsgId, WARNING_MSG_NONE);
    EXPECT_FALSE(output.eventLog);
}
