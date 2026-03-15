/**
 * @file test_door_ecu_command_handler.cpp
 * @brief Unit tests for F-03 DoorEcuCommandHandler.
 *
 * @details These tests define expected ACK/retry/DTC behavior according to
 *          SDD-F-03 and UC exception scenarios.
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */

#include <gtest/gtest.h>
#include <array>
#include <cstring>

extern "C" {
#include "childlock/door_ecu_command_handler.h"
}

class DoorEcuCommandHandlerTest : public ::testing::Test
{
protected:
    DoorEcuCommandHandler_Input_t input;
    DoorEcuCommandHandler_Output_t output;

    void SetUp() override
    {
        input.targetClState = CL_STATE_ON;
        input.decisionReason = DECH_REASON_DRIVER_REQUEST;
        input.doorEcuAck = true;
    }
};

TEST_F(DoorEcuCommandHandlerTest, AckReceivedWithOnTargetProducesOnCommand)
{
    input.targetClState = CL_STATE_ON;
    input.doorEcuAck = true;

    const bool handled = DoorEcuCommandHandler_HandleCommand(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_EQ(DECH_COMMAND_CL_ON, output.childLockCommand);
    EXPECT_EQ(DECH_ACK_STATUS_RECEIVED, output.ackStatus);
    EXPECT_EQ(DECH_RETRY_STATUS_NOT_REQUIRED, output.retryStatus);
    EXPECT_EQ(FAULT_FLAG_NONE, output.dtc);
    EXPECT_EQ(0U, output.retryCount);
}

TEST_F(DoorEcuCommandHandlerTest, AckReceivedWithOffTargetProducesOffCommand)
{
    input.targetClState = CL_STATE_OFF;
    input.doorEcuAck = true;

    const bool handled = DoorEcuCommandHandler_HandleCommand(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_EQ(DECH_COMMAND_CL_OFF, output.childLockCommand);
    EXPECT_EQ(DECH_ACK_STATUS_RECEIVED, output.ackStatus);
    EXPECT_EQ(DECH_RETRY_STATUS_NOT_REQUIRED, output.retryStatus);
    EXPECT_EQ(FAULT_FLAG_NONE, output.dtc);
    EXPECT_EQ(0U, output.retryCount);
}

TEST_F(DoorEcuCommandHandlerTest, MissingAckExhaustsRetryAndSetsDtc)
{
    input.targetClState = CL_STATE_ON;
    input.doorEcuAck = false;

    const bool handled = DoorEcuCommandHandler_HandleCommand(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_EQ(DECH_COMMAND_CL_ON, output.childLockCommand);
    EXPECT_EQ(DECH_ACK_STATUS_NOT_RECEIVED, output.ackStatus);
    EXPECT_EQ(DECH_RETRY_STATUS_EXHAUSTED, output.retryStatus);
    EXPECT_EQ(FAULT_FLAG_ECU_ACK_FAIL, output.dtc);
    EXPECT_EQ(MAX_ECU_RETRY_COUNT, output.retryCount);
}

TEST_F(DoorEcuCommandHandlerTest, EmergencyReleaseStillUsesAckRetryPolicy)
{
    input.targetClState = CL_STATE_OFF;
    input.decisionReason = DECH_REASON_EMERGENCY_RELEASE;
    input.doorEcuAck = false;

    const bool handled = DoorEcuCommandHandler_HandleCommand(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_EQ(DECH_COMMAND_CL_OFF, output.childLockCommand);
    EXPECT_EQ(DECH_ACK_STATUS_NOT_RECEIVED, output.ackStatus);
    EXPECT_EQ(DECH_RETRY_STATUS_EXHAUSTED, output.retryStatus);
    EXPECT_EQ(FAULT_FLAG_ECU_ACK_FAIL, output.dtc);
    EXPECT_EQ(MAX_ECU_RETRY_COUNT, output.retryCount);
}

TEST_F(DoorEcuCommandHandlerTest, InvalidTargetStateIsRejected)
{
    std::array<unsigned char, sizeof(ChildLockState_t)> invalidRawState;
    invalidRawState.fill(0xFFU);

    std::memcpy(&input.targetClState, invalidRawState.data(), sizeof(input.targetClState));
    input.doorEcuAck = true;

    const bool handled = DoorEcuCommandHandler_HandleCommand(&input, &output);

    EXPECT_FALSE(handled);
}

TEST_F(DoorEcuCommandHandlerTest, NullInputIsRejected)
{
    const bool handled = DoorEcuCommandHandler_HandleCommand(nullptr, &output);

    EXPECT_FALSE(handled);
}

TEST_F(DoorEcuCommandHandlerTest, NullOutputIsRejected)
{
    const bool handled = DoorEcuCommandHandler_HandleCommand(&input, nullptr);

    EXPECT_FALSE(handled);
}
