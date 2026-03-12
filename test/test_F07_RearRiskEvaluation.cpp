/**
 * @file    test_F07_RearRiskEvaluation.cpp
 * @brief   Unit tests for F-07: Rear Risk Evaluation Module
 *
 * @details Tests cover covering positive cases, boundary conditions,
 *          fault injection (sensor faults, invalid inputs), and requirement-based
 *          scenarios per Failure_Mode.md.
 *          Achieves high Statement, Branch and MC/DC coverage.
 *
 * @version 1.1.0
 * @date    2026-03-11 01:42 (KST)
 * @author  AI Model (v1.0.0): Gemini 3.5 Pro — Initial implementation
 * @author  AI Model (v1.1.0): Claude Sonnet 4.6 (Thinking) — Review & fix
 * @copyright Synetics 20 CopyrightⓒSynetics_
 * @note    v1.1.0: Added Boundary_RiskLevelNone test case for 100% line coverage.
 */

#include <gtest/gtest.h>
extern "C" {
#include "F07_RearRiskEvaluation.h"
#include "childlock_types.h"
}

/**
 * @brief Test fixture for F-07 using Google Test
 */
class F07RearRiskEvaluationTest : public ::testing::Test {
protected:
    F07_Context_t ctx;
    RearRiskInput_t input;
    RearRiskOutput_t output;

    void SetUp() override {
        // Init context
        F07_RearRiskEvaluation_Init(&ctx);

        // Set default valid, safe input
        input.distanceM = 10.0f;
        input.relSpeedMps = 0.0f;
        input.sensorHealth = true;
        input.sensorConfidence = 0.9f;
        input.timestampMs = 1000U;

        // Reset output
        output.riskHigh = false;
        output.riskLevel = RISK_LEVEL_NONE;
        output.riskValid = false;
        output.faultFlag = FAULT_FLAG_NONE;
    }

    void TearDown() override {
        F07_RearRiskEvaluation_Reset(&ctx);
    }
};

/* =========================================================================
 * 1. Positive & Hysteresis Scenarios
 * ========================================================================= */

TEST_F(F07RearRiskEvaluationTest, Positive_NormalSafeState) {
    input.distanceM = 5.0f;
    input.relSpeedMps = 1.0f;

    F07_RearRiskEvaluation_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.riskValid);
    EXPECT_FALSE(output.riskHigh);
    EXPECT_EQ(output.riskLevel, RISK_LEVEL_LOW);
    EXPECT_EQ(output.faultFlag, FAULT_FLAG_NONE);
}

TEST_F(F07RearRiskEvaluationTest, Positive_HighRiskWithHysteresis) {
    // 1st cycle: Threat detected but not confirmed (Hysteresis ON delay)
    input.distanceM = 2.0f; // <= 3.0
    input.relSpeedMps = 3.0f; // >= 2.0
    input.timestampMs = 1000U;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    EXPECT_TRUE(output.riskValid);
    EXPECT_FALSE(output.riskHigh); // Not yet confirmed
    EXPECT_EQ(output.riskLevel, RISK_LEVEL_HIGH); // Raw level is HIGH

    // 2nd cycle: Threat persists, past CONFIRM duration
    input.timestampMs = 1000U + REAR_RISK_CONFIRM_DURATION_MS;
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    EXPECT_TRUE(output.riskHigh); // Confirmed!

    // 3rd cycle: Threat goes away, but not cleared (Hysteresis OFF delay)
    input.distanceM = 5.0f; // Safe distance
    input.timestampMs = 2000U; // Start clear delay
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    EXPECT_TRUE(output.riskHigh); // Still HIGH due to delay
    EXPECT_EQ(output.riskLevel, RISK_LEVEL_LOW); // Raw level is LOW

    // 4th cycle: Threat clear time expires
    input.timestampMs = 2000U + REAR_RISK_CLEAR_DURATION_MS;
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    EXPECT_FALSE(output.riskHigh); // Cleared!
}

/* =========================================================================
 * 2. Boundary Scenarios
 * ========================================================================= */

TEST_F(F07RearRiskEvaluationTest, Boundary_RiskThresholds) {
    // Exactly at thresholds
    input.distanceM = REAR_RISK_DIST_THRESHOLD_M;
    input.relSpeedMps = REAR_RISK_RELSPEED_THRESHOLD_MPS;
    input.timestampMs = 1000U;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    EXPECT_EQ(output.riskLevel, RISK_LEVEL_HIGH);

    input.timestampMs += REAR_RISK_CONFIRM_DURATION_MS;
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    EXPECT_TRUE(output.riskHigh);
}

TEST_F(F07RearRiskEvaluationTest, Boundary_MediumRisk_FastButFar) {
    input.distanceM = REAR_RISK_DIST_THRESHOLD_M + 1.0f;
    input.relSpeedMps = REAR_RISK_RELSPEED_THRESHOLD_MPS;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    EXPECT_EQ(output.riskLevel, RISK_LEVEL_MEDIUM);
}

TEST_F(F07RearRiskEvaluationTest, Boundary_MediumRisk_CloseButSlow) {
    input.distanceM = REAR_RISK_DIST_THRESHOLD_M;
    input.relSpeedMps = REAR_RISK_RELSPEED_THRESHOLD_MPS - 0.5f;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    EXPECT_EQ(output.riskLevel, RISK_LEVEL_MEDIUM);
}

/* =========================================================================
 * 3. Fault Injection Scenarios (Failure Mode Handling)
 * ========================================================================= */

TEST_F(F07RearRiskEvaluationTest, Fault_SensorHealthFalse) {
    input.sensorHealth = false;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    
    EXPECT_FALSE(output.riskValid);
    EXPECT_FALSE(output.riskHigh);
    EXPECT_EQ(output.faultFlag, FAULT_FLAG_REAR_SENSOR);
}

TEST_F(F07RearRiskEvaluationTest, Fault_LowConfidence) {
    input.sensorConfidence = REAR_RISK_SENSOR_CONFIDENCE_MIN - 0.1f;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    
    EXPECT_FALSE(output.riskValid);
    EXPECT_EQ(output.faultFlag, FAULT_FLAG_REAR_SENSOR);
}

TEST_F(F07RearRiskEvaluationTest, Fault_InvalidDistance_Negative) {
    input.distanceM = -1.0f;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    
    EXPECT_FALSE(output.riskValid);
    EXPECT_EQ(output.faultFlag, FAULT_FLAG_REAR_SENSOR);
}

TEST_F(F07RearRiskEvaluationTest, Fault_InvalidDistance_TooLarge) {
    input.distanceM = REAR_RISK_DIST_MAX_VALID_M + 1.0f;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    
    EXPECT_FALSE(output.riskValid);
}

TEST_F(F07RearRiskEvaluationTest, Fault_InvalidSpeed_Negative) {
    input.relSpeedMps = -1.0f;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    
    EXPECT_FALSE(output.riskValid);
}

TEST_F(F07RearRiskEvaluationTest, Fault_InvalidSpeed_TooLarge) {
    input.relSpeedMps = REAR_RISK_RELSPEED_MAX_VALID_MPS + 1.0f;
    
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    
    EXPECT_FALSE(output.riskValid);
}

/* =========================================================================
 * 4. Requirement-based / Robustness Scenarios
 * ========================================================================= */

TEST_F(F07RearRiskEvaluationTest, Robustness_NullPointers) {
    // Should not crash when evaluating with NULL
    F07_RearRiskEvaluation_Run(NULL, &input, &output);
    F07_RearRiskEvaluation_Run(&ctx, NULL, &output);
    F07_RearRiskEvaluation_Run(&ctx, &input, NULL);
    
    // Explicit null reset init check
    F07_RearRiskEvaluation_Init(NULL);
    F07_RearRiskEvaluation_Reset(NULL);
    
    // No explicit EXPECT here, passing without crash is the success.
    SUCCEED();
}

TEST_F(F07RearRiskEvaluationTest, Robustness_AutoInit) {
    // If we deliberately un-init the context
    ctx.isInitialized = false;
    ctx.isRiskHighActive = true; // Mess up the state
    
    // Valid input
    input.distanceM = 10.0f;
    input.relSpeedMps = 0.0f;

    // The Run function should auto-init the context
    F07_RearRiskEvaluation_Run(&ctx, &input, &output);
    
    EXPECT_TRUE(ctx.isInitialized);
    EXPECT_FALSE(ctx.isRiskHighActive); // Auto-init should have cleared this
    EXPECT_FALSE(output.riskHigh);
}

/* =========================================================================
 * 5. Risk Level NONE Coverage (v1.1.0 addition)
 * ========================================================================= */

/**
 * @brief Verifies RISK_LEVEL_NONE is returned when object is very far away.
 * @note  Covers the final return branch in classifyRiskLevel() for 100% line coverage.
 */
TEST_F(F07RearRiskEvaluationTest, Boundary_RiskLevelNone_VeryFarObject) {
    /* Distance well beyond 2x threshold (> 6.0m) and zero approach speed */
    input.distanceM   = REAR_RISK_DIST_THRESHOLD_M * 3.0f; /* 9.0m - far beyond any threshold */
    input.relSpeedMps = 0.0f;

    F07_RearRiskEvaluation_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.riskValid);
    EXPECT_FALSE(output.riskHigh);
    EXPECT_EQ(output.riskLevel, RISK_LEVEL_NONE);
    EXPECT_EQ(output.faultFlag, FAULT_FLAG_NONE);
}
