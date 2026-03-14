/**
 * @file    test_F01_InputMonitorAndValidator.cpp
 * @brief   Unit tests for F-01: Input Monitor and Validator Module
 *
 * @details Tests cover positive cases, boundary conditions, fault injection
 *          (sensor faults, invalid inputs, NULL pointers), and requirement-based
 *          scenarios per Failure_Mode.md and SDD 3.1.
 *          Achieves high Statement, Branch and MC/DC coverage.
 *
 * @version 1.0.0
 * @date    2026-03-13
 * @author  AI Model (v1.0.0): Claude Opus 4.6
 * @copyright Synetics 20 CopyrightcSynetics_
 *
 * @req_id      REQ-SW-CL-001, REQ-SW-CL-002, REQ-SW-CL-003, REQ-SW-CL-004
 * @traceability DD-CL-F01, UC-1, UC-2, UC-3, UC-4
 */

#include <gtest/gtest.h>
extern "C" {
#include "F01_InputMonitorAndValidator.h"
#include "childlock_types.h"
}

/**
 * @brief Test fixture for F-01 InputMonitorAndValidator using Google Test
 */
class F01InputMonitorAndValidatorTest : public ::testing::Test {
protected:
    F01_Context_t ctx;
    F01_RawInput_t input;
    F01_ValidatedOutput_t output;

    void SetUp() override {
        /* Initialize context */
        F01_InputMonitorAndValidator_Init(&ctx);

        /* Default: valid, quiet input (IGN ON, zero speed, no events) */
        input.switchRequest           = SWITCH_REQUEST_NONE;
        input.vehicleSpeedKmh         = 0.0f;
        input.speedTimestampMs        = 1000U;
        input.currentTimestampMs      = 1000U; /* No elapsed time = fresh signal */
        input.crashSignalActive       = false;
        input.crashSignalValid        = false;
        input.rearDoorInnerHandleActive = false;
        input.ignitionState           = IGN_STATE_ON;

        /* Clear output */
        output.validSwitchInput    = SWITCH_REQUEST_NONE;
        output.validSpeedKmh       = 0.0f;
        output.speedValid          = false;
        output.validCrashSignal    = false;
        output.autoLockConditionMet = false;
        output.eventFlags          = EVENT_FLAG_NONE;
        output.faultFlags          = FAULT_FLAG_NONE;
    }

    void TearDown() override {
        F01_InputMonitorAndValidator_Reset(&ctx);
    }
};

/* =========================================================================
 * 1. Positive Scenarios (Normal Operation)
 * ========================================================================= */

/**
 * @brief All inputs at default (zero/false) with IGN ON -> safe defaults, no faults.
 * @req_id REQ-SW-CL-001
 */
TEST_F(F01InputMonitorAndValidatorTest, Positive_NoInput_SafeDefaults) {
    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_EQ(output.validSwitchInput, SWITCH_REQUEST_NONE);
    EXPECT_FLOAT_EQ(output.validSpeedKmh, 0.0f);
    EXPECT_TRUE(output.speedValid);
    EXPECT_FALSE(output.validCrashSignal);
    EXPECT_FALSE(output.autoLockConditionMet);
    EXPECT_EQ(output.eventFlags, EVENT_FLAG_NONE);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_NONE);
}

/**
 * @brief Switch ON request is passed through to output.
 * @req_id REQ-SW-CL-001
 * @traceability UC-1
 */
TEST_F(F01InputMonitorAndValidatorTest, Positive_SwitchOnRequest) {
    input.switchRequest = SWITCH_REQUEST_ON;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_EQ(output.validSwitchInput, SWITCH_REQUEST_ON);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_NONE);
}

/**
 * @brief Switch OFF request is passed through to output.
 * @req_id REQ-SW-CL-001
 * @traceability UC-1
 */
TEST_F(F01InputMonitorAndValidatorTest, Positive_SwitchOffRequest) {
    input.switchRequest = SWITCH_REQUEST_OFF;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_EQ(output.validSwitchInput, SWITCH_REQUEST_OFF);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_NONE);
}

/**
 * @brief Normal speed (60 km/h) with fresh timestamp passes validation.
 * @req_id REQ-SW-CL-002
 */
TEST_F(F01InputMonitorAndValidatorTest, Positive_ValidSpeed_Normal) {
    input.vehicleSpeedKmh = 60.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.speedValid);
    EXPECT_FLOAT_EQ(output.validSpeedKmh, 60.0f);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_NONE);
}

/**
 * @brief Crash signal with both active AND valid flags -> validCrashSignal = true.
 * @req_id REQ-SW-CL-003
 * @traceability UC-3
 */
TEST_F(F01InputMonitorAndValidatorTest, Positive_ValidCrashSignal) {
    input.crashSignalActive = true;
    input.crashSignalValid  = true;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.validCrashSignal);
}

/**
 * @brief Rear door inner handle activation sets EVENT_FLAG_REAR_DOOR_HANDLE.
 * @req_id REQ-SW-CL-004
 * @traceability UC-4
 */
TEST_F(F01InputMonitorAndValidatorTest, Positive_RearDoorHandleEvent) {
    input.rearDoorInnerHandleActive = true;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_EQ(output.eventFlags, EVENT_FLAG_REAR_DOOR_HANDLE);
}

/**
 * @brief Speed above threshold (5 km/h > 3 km/h) sets autoLockConditionMet.
 * @req_id REQ-SW-CL-002
 * @traceability UC-2
 */
TEST_F(F01InputMonitorAndValidatorTest, Positive_AutoLockCondition_SpeedAboveThreshold) {
    input.vehicleSpeedKmh = 5.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.speedValid);
    EXPECT_TRUE(output.autoLockConditionMet);
}

/**
 * @brief All inputs active simultaneously - verify all outputs set correctly.
 * @req_id REQ-SW-CL-001, REQ-SW-CL-002, REQ-SW-CL-003, REQ-SW-CL-004
 */
TEST_F(F01InputMonitorAndValidatorTest, Positive_AllInputsSimultaneous) {
    input.switchRequest             = SWITCH_REQUEST_ON;
    input.vehicleSpeedKmh           = 50.0f;
    input.crashSignalActive         = true;
    input.crashSignalValid          = true;
    input.rearDoorInnerHandleActive = true;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_EQ(output.validSwitchInput, SWITCH_REQUEST_ON);
    EXPECT_FLOAT_EQ(output.validSpeedKmh, 50.0f);
    EXPECT_TRUE(output.speedValid);
    EXPECT_TRUE(output.validCrashSignal);
    EXPECT_TRUE(output.autoLockConditionMet);
    EXPECT_EQ(output.eventFlags, EVENT_FLAG_REAR_DOOR_HANDLE);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_NONE);
}

/* =========================================================================
 * 2. Boundary Scenarios
 * ========================================================================= */

/**
 * @brief Speed = 0.0 km/h is valid, autoLockConditionMet = false.
 * @req_id REQ-SW-CL-002
 */
TEST_F(F01InputMonitorAndValidatorTest, Boundary_SpeedAtZero) {
    input.vehicleSpeedKmh = 0.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.speedValid);
    EXPECT_FLOAT_EQ(output.validSpeedKmh, 0.0f);
    EXPECT_FALSE(output.autoLockConditionMet);
}

/**
 * @brief Speed exactly at threshold (3.0 km/h) does NOT trigger auto-lock (> not >=).
 * @req_id REQ-SW-CL-002
 * @traceability UC-2, D6
 */
TEST_F(F01InputMonitorAndValidatorTest, Boundary_SpeedAtExactThreshold) {
    input.vehicleSpeedKmh = 3.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.speedValid);
    EXPECT_FALSE(output.autoLockConditionMet);
}

/**
 * @brief Speed just above threshold (3.1 km/h) triggers auto-lock.
 * @req_id REQ-SW-CL-002
 * @traceability UC-2, D6
 */
TEST_F(F01InputMonitorAndValidatorTest, Boundary_SpeedJustAboveThreshold) {
    input.vehicleSpeedKmh = 3.1f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.speedValid);
    EXPECT_TRUE(output.autoLockConditionMet);
}

/**
 * @brief Speed at max valid (250 km/h) passes validation.
 * @req_id REQ-SW-CL-001
 */
TEST_F(F01InputMonitorAndValidatorTest, Boundary_SpeedAtMax) {
    input.vehicleSpeedKmh = 250.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.speedValid);
    EXPECT_FLOAT_EQ(output.validSpeedKmh, 250.0f);
}

/**
 * @brief Speed just above max (250.1 km/h) triggers FAULT_FLAG_SPEED_FAULT.
 * @req_id REQ-SW-CL-001
 */
TEST_F(F01InputMonitorAndValidatorTest, Boundary_SpeedJustAboveMax) {
    input.vehicleSpeedKmh = 250.1f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_FALSE(output.speedValid);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_SPEED_FAULT);
    EXPECT_FLOAT_EQ(output.validSpeedKmh, 0.0f);
}

/**
 * @brief Speed signal timeout at 499ms (just under limit) -> valid.
 * @req_id REQ-SW-CL-002
 * @traceability D2
 */
TEST_F(F01InputMonitorAndValidatorTest, Boundary_SpeedTimeoutJustBefore) {
    input.speedTimestampMs   = 501U;
    input.currentTimestampMs = 1000U; /* elapsed = 499ms, just within limit */
    input.vehicleSpeedKmh    = 60.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.speedValid);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_NONE);
}

/**
 * @brief Speed signal timeout at exactly 500ms (at limit) -> FAULT.
 *        elapsed >= SPEED_SIGNAL_TIMEOUT_MS is NOT valid.
 * @req_id REQ-SW-CL-002
 * @traceability D2
 */
TEST_F(F01InputMonitorAndValidatorTest, Boundary_SpeedTimeoutAtBoundary) {
    input.speedTimestampMs   = 500U;
    input.currentTimestampMs = 1000U; /* elapsed = 500ms, exactly at limit */
    input.vehicleSpeedKmh    = 60.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_FALSE(output.speedValid);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_SPEED_FAULT);
    EXPECT_FLOAT_EQ(output.validSpeedKmh, 0.0f);
}

/* =========================================================================
 * 3. Fault Injection Scenarios (Negative / Failure Modes)
 * ========================================================================= */

/**
 * @brief Negative speed value (-1.0) triggers FAULT_FLAG_SPEED_FAULT.
 * @req_id REQ-SW-CL-002
 */
TEST_F(F01InputMonitorAndValidatorTest, Fault_SpeedNegative) {
    input.vehicleSpeedKmh = -1.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_FALSE(output.speedValid);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_SPEED_FAULT);
    EXPECT_FLOAT_EQ(output.validSpeedKmh, 0.0f);
}

/**
 * @brief Speed far above max (300 km/h) triggers FAULT_FLAG_SPEED_FAULT.
 * @req_id REQ-SW-CL-002
 */
TEST_F(F01InputMonitorAndValidatorTest, Fault_SpeedFarAboveMax) {
    input.vehicleSpeedKmh = 300.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_FALSE(output.speedValid);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_SPEED_FAULT);
}

/**
 * @brief Speed signal timed out (1000ms elapsed > 500ms limit).
 * @req_id REQ-SW-CL-002
 * @traceability UC-2
 */
TEST_F(F01InputMonitorAndValidatorTest, Fault_SpeedTimeout) {
    input.speedTimestampMs   = 0U;
    input.currentTimestampMs = 1000U; /* 1000ms elapsed */
    input.vehicleSpeedKmh    = 60.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_FALSE(output.speedValid);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_SPEED_FAULT);
}

/**
 * @brief Crash signal active but NOT valid -> validCrashSignal = false (no false alarm).
 * @req_id REQ-SW-CL-003
 * @traceability UC-3
 */
TEST_F(F01InputMonitorAndValidatorTest, Fault_CrashActiveButNotValid) {
    input.crashSignalActive = true;
    input.crashSignalValid  = false;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_FALSE(output.validCrashSignal);
}

/**
 * @brief Crash signal valid but NOT active -> validCrashSignal = false.
 * @req_id REQ-SW-CL-003
 * @traceability UC-3
 */
TEST_F(F01InputMonitorAndValidatorTest, Fault_CrashValidButNotActive) {
    input.crashSignalActive = false;
    input.crashSignalValid  = true;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_FALSE(output.validCrashSignal);
}

/**
 * @brief IGN OFF -> all outputs are safe defaults regardless of active inputs.
 * @req_id REQ-SW-CL-001
 */
TEST_F(F01InputMonitorAndValidatorTest, Fault_IgnitionOff) {
    input.ignitionState             = IGN_STATE_OFF;
    input.switchRequest             = SWITCH_REQUEST_ON;
    input.crashSignalActive         = true;
    input.crashSignalValid          = true;
    input.vehicleSpeedKmh           = 60.0f;
    input.rearDoorInnerHandleActive = true;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    /* All outputs must be safe defaults when IGN OFF */
    EXPECT_EQ(output.validSwitchInput, SWITCH_REQUEST_NONE);
    EXPECT_FLOAT_EQ(output.validSpeedKmh, 0.0f);
    EXPECT_FALSE(output.speedValid);
    EXPECT_FALSE(output.validCrashSignal);
    EXPECT_FALSE(output.autoLockConditionMet);
    EXPECT_EQ(output.eventFlags, EVENT_FLAG_NONE);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_NONE);
}

/**
 * @brief NULL pointer arguments must not crash. Defensive programming check.
 * @req_id REQ-SW-CL-001
 */
TEST_F(F01InputMonitorAndValidatorTest, Fault_NullPointers) {
    /* None of these should crash */
    F01_InputMonitorAndValidator_Run(NULL, &input, &output);
    F01_InputMonitorAndValidator_Run(&ctx, NULL, &output);
    F01_InputMonitorAndValidator_Run(&ctx, &input, NULL);
    F01_InputMonitorAndValidator_Run(NULL, NULL, NULL);
    F01_InputMonitorAndValidator_Init(NULL);
    F01_InputMonitorAndValidator_Reset(NULL);

    SUCCEED();
}

/* =========================================================================
 * 4. Requirement-Based / UC Traceability Scenarios
 * ========================================================================= */

/**
 * @brief UC-1: Switch OFF request passes through while driving.
 *        F-01 does NOT inhibit unlock; that responsibility belongs to F-02.
 * @req_id REQ-SW-CL-001
 * @traceability UC-1
 */
TEST_F(F01InputMonitorAndValidatorTest, UC1_SwitchOffPassthroughWhileDriving) {
    input.switchRequest   = SWITCH_REQUEST_OFF;
    input.vehicleSpeedKmh = 50.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    /* F-01 passes through the OFF request; F-02 handles inhibition */
    EXPECT_EQ(output.validSwitchInput, SWITCH_REQUEST_OFF);
    EXPECT_TRUE(output.speedValid);
    EXPECT_TRUE(output.autoLockConditionMet);
}

/**
 * @brief UC-2: Speed fault (timeout) raises FAULT_FLAG_SPEED_FAULT,
 *        autoLockConditionMet must be false (cannot auto-lock with bad speed).
 * @req_id REQ-SW-CL-002
 * @traceability UC-2
 */
TEST_F(F01InputMonitorAndValidatorTest, UC2_SpeedFaultRaisesFaultFlag) {
    input.vehicleSpeedKmh    = 60.0f;
    input.speedTimestampMs   = 0U;
    input.currentTimestampMs = 1000U; /* 1000ms elapsed > 500ms timeout */

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_FALSE(output.speedValid);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_SPEED_FAULT);
    EXPECT_FALSE(output.autoLockConditionMet);
}

/**
 * @brief UC-2: Valid speed above threshold correctly triggers auto-lock condition.
 * @req_id REQ-SW-CL-002
 * @traceability UC-2
 */
TEST_F(F01InputMonitorAndValidatorTest, UC2_AutoLockConditionWithValidSpeed) {
    input.vehicleSpeedKmh = 10.0f;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.speedValid);
    EXPECT_TRUE(output.autoLockConditionMet);
}

/**
 * @brief UC-3: Crash validation requires both active + valid flags.
 * @req_id REQ-SW-CL-003
 * @traceability UC-3
 */
TEST_F(F01InputMonitorAndValidatorTest, UC3_CrashValidationBothFlags) {
    input.crashSignalActive = true;
    input.crashSignalValid  = true;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(output.validCrashSignal);
}

/**
 * @brief UC-4: Rear door handle event sets EVENT_FLAG_REAR_DOOR_HANDLE.
 * @req_id REQ-SW-CL-004
 * @traceability UC-4
 */
TEST_F(F01InputMonitorAndValidatorTest, UC4_RearDoorHandleEventFlag) {
    input.rearDoorInnerHandleActive = true;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_EQ(output.eventFlags, EVENT_FLAG_REAR_DOOR_HANDLE);
}

/* =========================================================================
 * 5. Robustness / Edge Cases
 * ========================================================================= */

/**
 * @brief Uninitialized context auto-initializes and produces valid output.
 * @req_id REQ-SW-CL-001
 */
TEST_F(F01InputMonitorAndValidatorTest, Robustness_AutoInit) {
    ctx.isInitialized = false;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_TRUE(ctx.isInitialized);
    /* Default input has fresh zero speed = valid */
    EXPECT_TRUE(output.speedValid);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_NONE);
}

/**
 * @brief No handle event when rearDoorInnerHandleActive is false.
 * @req_id REQ-SW-CL-004
 */
TEST_F(F01InputMonitorAndValidatorTest, Robustness_NoHandleEvent) {
    input.rearDoorInnerHandleActive = false;

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_EQ(output.eventFlags, EVENT_FLAG_NONE);
}

/**
 * @brief Speed fault with speed = 0 but stale timestamp -> fault detected.
 *        Ensures timeout check is not bypassed by zero speed.
 * @req_id REQ-SW-CL-002
 */
TEST_F(F01InputMonitorAndValidatorTest, Robustness_ZeroSpeedButStaleTimestamp) {
    input.vehicleSpeedKmh    = 0.0f;
    input.speedTimestampMs   = 0U;
    input.currentTimestampMs = 600U; /* 600ms elapsed > 500ms limit */

    F01_InputMonitorAndValidator_Run(&ctx, &input, &output);

    EXPECT_FALSE(output.speedValid);
    EXPECT_EQ(output.faultFlags, FAULT_FLAG_SPEED_FAULT);
}
