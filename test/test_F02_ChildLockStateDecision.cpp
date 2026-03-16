/**
 * @file test_F02_ChildLockStateDecision.cpp
 * @brief Unit tests for F-02: Child Lock State Decision (GoogleTest)
 *
 * @details Tests cover all priority paths: crash > fault > driver input > auto-lock,
 *          plus boundary conditions for unlock inhibition and state change detection.
 *
 * @traceability FG-01, F-02, SwDD 5.1.2, SwDD 5.1.3
 * @req_id UC-1, UC-2, UC-3
 */

#include <gtest/gtest.h>
#include "F02_ChildLockStateDecision.h"

class F02ChildLockStateDecisionTest : public ::testing::Test
{
protected:
    DecisionInput MakeBaseInput()
    {
        DecisionInput input;
        input.validSwitchInput   = DRIVER_NONE;
        input.vehicleSpeedKmh    = 0.0F;
        input.validCrashSignal   = false;
        input.currentClState     = CL_OFF;
        input.faultFlags         = FaultFlags{false, false, false};
        input.autoLockConditionMet = false;
        return input;
    }
};

/* =========================================================================
 * 1. Priority: Crash (highest)
 * ========================================================================= */

TEST_F(F02ChildLockStateDecisionTest, Crash_HasHighestPriority_OverFaultAndDriver)
{
    DecisionInput input = MakeBaseInput();
    input.validCrashSignal   = true;
    input.validSwitchInput   = DRIVER_REQ_ON;
    input.faultFlags         = FaultFlags{true, true, true};
    input.autoLockConditionMet = true;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,   CL_OFF);
    EXPECT_EQ(result.decisionReason,  DECISION_CRASH_DETECTED);
    EXPECT_EQ(result.safetyAction,    SAFETY_ENTER_EMERGENCY_RELEASED);
    EXPECT_FALSE(result.stateChanged); /* CL was OFF, stays OFF */
}

TEST_F(F02ChildLockStateDecisionTest, Crash_WhenClWasOn_StateChangedTrue)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState   = CL_ON;
    input.validCrashSignal = true;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_OFF);
    EXPECT_EQ(result.decisionReason, DECISION_CRASH_DETECTED);
    EXPECT_EQ(result.safetyAction,   SAFETY_ENTER_EMERGENCY_RELEASED);
    EXPECT_TRUE(result.stateChanged); /* CL was ON, now OFF */
}

/* =========================================================================
 * 2. Priority: Fault
 * ========================================================================= */

TEST_F(F02ChildLockStateDecisionTest, Fault_HasPriorityOverDriverInput)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState     = CL_OFF;
    input.validSwitchInput   = DRIVER_REQ_OFF;
    input.faultFlags.sensorFault = true;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_ON);
    EXPECT_EQ(result.decisionReason, DECISION_FAULT_DETECTED);
    EXPECT_EQ(result.safetyAction,   SAFETY_ENTER_SAFE_LOCKED);
    EXPECT_TRUE(result.stateChanged);
}

TEST_F(F02ChildLockStateDecisionTest, Fault_SpeedSignalFault_SafeLocked)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState            = CL_OFF;
    input.faultFlags.speedSignalFault = true;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_ON);
    EXPECT_EQ(result.decisionReason, DECISION_FAULT_DETECTED);
    EXPECT_EQ(result.safetyAction,   SAFETY_ENTER_SAFE_LOCKED);
    EXPECT_TRUE(result.stateChanged);
}

TEST_F(F02ChildLockStateDecisionTest, Fault_StateInconsistency_SafeLocked)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState              = CL_OFF;
    input.faultFlags.stateInconsistency = true;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_ON);
    EXPECT_EQ(result.decisionReason, DECISION_FAULT_DETECTED);
    EXPECT_EQ(result.safetyAction,   SAFETY_ENTER_SAFE_LOCKED);
}

/* =========================================================================
 * 3. Priority: Driver Input
 * ========================================================================= */

TEST_F(F02ChildLockStateDecisionTest, Driver_OnRequest_ActivatesLock)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState   = CL_OFF;
    input.validSwitchInput = DRIVER_REQ_ON;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_ON);
    EXPECT_EQ(result.decisionReason, DECISION_DRIVER_LOCK_REQUEST);
    EXPECT_EQ(result.safetyAction,   SAFETY_NONE);
    EXPECT_TRUE(result.stateChanged);
}

TEST_F(F02ChildLockStateDecisionTest, Driver_OffRequest_AtZeroSpeed_Allowed)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState   = CL_ON;
    input.validSwitchInput = DRIVER_REQ_OFF;
    input.vehicleSpeedKmh  = 0.0F;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_OFF);
    EXPECT_EQ(result.decisionReason, DECISION_DRIVER_UNLOCK_REQUEST);
    EXPECT_EQ(result.safetyAction,   SAFETY_NONE);
    EXPECT_TRUE(result.stateChanged);
}

TEST_F(F02ChildLockStateDecisionTest, Driver_OffRequest_AtThreshold_Allowed)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState   = CL_ON;
    input.validSwitchInput = DRIVER_REQ_OFF;
    input.vehicleSpeedKmh  = 3.0F;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_OFF);
    EXPECT_EQ(result.decisionReason, DECISION_DRIVER_UNLOCK_REQUEST);
    EXPECT_EQ(result.safetyAction,   SAFETY_NONE);
    EXPECT_TRUE(result.stateChanged);
}

TEST_F(F02ChildLockStateDecisionTest, Driver_OffRequest_AboveThreshold_Inhibited)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState   = CL_ON;
    input.validSwitchInput = DRIVER_REQ_OFF;
    input.vehicleSpeedKmh  = 3.1F;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_ON);
    EXPECT_EQ(result.decisionReason, DECISION_UNLOCK_INHIBITED_WHILE_MOVING);
    EXPECT_EQ(result.safetyAction,   SAFETY_INHIBIT_UNLOCK);
    EXPECT_FALSE(result.stateChanged);
}

/* =========================================================================
 * 4. Priority: Auto-lock
 * ========================================================================= */

TEST_F(F02ChildLockStateDecisionTest, AutoLock_ActivatesLockWhenClOff)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState      = CL_OFF;
    input.autoLockConditionMet = true;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_ON);
    EXPECT_EQ(result.decisionReason, DECISION_AUTO_LOCK_CONDITION_MET);
    EXPECT_EQ(result.safetyAction,   SAFETY_NONE);
    EXPECT_TRUE(result.stateChanged);
}

TEST_F(F02ChildLockStateDecisionTest, AutoLock_WhenClAlreadyOn_StateChangedFalse)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState      = CL_ON;
    input.autoLockConditionMet = true;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_ON);
    EXPECT_EQ(result.decisionReason, DECISION_AUTO_LOCK_CONDITION_MET);
    EXPECT_FALSE(result.stateChanged); /* No change: was ON, stays ON */
}

/* =========================================================================
 * 5. No conditions (hold current state)
 * ========================================================================= */

TEST_F(F02ChildLockStateDecisionTest, NoConditions_ClOn_HoldsState)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = CL_ON;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_ON);
    EXPECT_EQ(result.decisionReason, DECISION_NO_STATE_CHANGE);
    EXPECT_EQ(result.safetyAction,   SAFETY_NONE);
    EXPECT_FALSE(result.stateChanged);
}

TEST_F(F02ChildLockStateDecisionTest, NoConditions_ClOff_HoldsState)
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = CL_OFF;

    DecisionResult result = DecideChildLockState(&input);

    EXPECT_EQ(result.targetClState,  CL_OFF);
    EXPECT_EQ(result.decisionReason, DECISION_NO_STATE_CHANGE);
    EXPECT_EQ(result.safetyAction,   SAFETY_NONE);
    EXPECT_FALSE(result.stateChanged);
}
