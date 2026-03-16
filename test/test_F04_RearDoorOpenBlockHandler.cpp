/**
 * @file test_F04_RearDoorOpenBlockHandler.cpp
 * @brief Unit tests for F-04 RearDoorOpenBlockHandler.
 *
 * @details Tests cover UC-4 positive, alternative, and fault-oriented
 *          behavior using the public F-04 API. All door feedback variants
 *          for CL ON are covered.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */

#include <gtest/gtest.h>
#include <array>
#include <cstring>

extern "C" {
#include "F04_RearDoorOpenBlockHandler.h"
}

class RearDoorOpenBlockHandlerTest : public ::testing::Test
{
protected:
    RearDoorOpenBlockHandler_Input_t input;
    RearDoorOpenBlockHandler_Output_t output;

    void SetUp() override
    {
        input.rearDoorInnerHandleEvent = false;
        input.currentClState = CL_STATE_OFF;
        input.doorStateFeedback = RDOBH_DOOR_FEEDBACK_CLOSED;
    }
};

/* =========================================================================
 * 1. No event
 * ========================================================================= */

TEST_F(RearDoorOpenBlockHandlerTest, NoHandleEventProducesNoAction)
{
    const bool handled = RearDoorOpenBlockHandler_HandleEvent(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_FALSE(output.openRequestBlock);
    EXPECT_EQ(RDOBH_DRIVER_NOTICE_NONE, output.driverNotice);
    EXPECT_EQ(RDOBH_RESULT_NO_EVENT, output.blockResult);
}

/* =========================================================================
 * 2. CL OFF → allowed
 * ========================================================================= */

TEST_F(RearDoorOpenBlockHandlerTest, HandleEventWithChildLockOffAllowsOpenRequest)
{
    input.rearDoorInnerHandleEvent = true;
    input.currentClState = CL_STATE_OFF;
    input.doorStateFeedback = RDOBH_DOOR_FEEDBACK_OPENED;

    const bool handled = RearDoorOpenBlockHandler_HandleEvent(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_FALSE(output.openRequestBlock);
    EXPECT_EQ(RDOBH_DRIVER_NOTICE_NONE, output.driverNotice);
    EXPECT_EQ(RDOBH_RESULT_ALLOWED, output.blockResult);
}

/* =========================================================================
 * 3. CL ON + various door feedback states
 * ========================================================================= */

TEST_F(RearDoorOpenBlockHandlerTest, HandleEventWithChildLockOnAndClosedFeedback_Blocked)
{
    input.rearDoorInnerHandleEvent = true;
    input.currentClState = CL_STATE_ON;
    input.doorStateFeedback = RDOBH_DOOR_FEEDBACK_CLOSED;

    const bool handled = RearDoorOpenBlockHandler_HandleEvent(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_TRUE(output.openRequestBlock);
    EXPECT_EQ(RDOBH_DRIVER_NOTICE_BLOCKED, output.driverNotice);
    EXPECT_EQ(RDOBH_RESULT_BLOCKED, output.blockResult);
}

TEST_F(RearDoorOpenBlockHandlerTest, HandleEventWithChildLockOnAndUnknownFeedback_Blocked)
{
    /* UNKNOWN feedback should NOT be treated as BLOCK_FAILED - remains BLOCKED */
    input.rearDoorInnerHandleEvent = true;
    input.currentClState = CL_STATE_ON;
    input.doorStateFeedback = RDOBH_DOOR_FEEDBACK_UNKNOWN;

    const bool handled = RearDoorOpenBlockHandler_HandleEvent(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_TRUE(output.openRequestBlock);
    EXPECT_EQ(RDOBH_DRIVER_NOTICE_BLOCKED, output.driverNotice);
    EXPECT_EQ(RDOBH_RESULT_BLOCKED, output.blockResult);
}

TEST_F(RearDoorOpenBlockHandlerTest, DoorOpenedDespiteBlockReportsFailure)
{
    input.rearDoorInnerHandleEvent = true;
    input.currentClState = CL_STATE_ON;
    input.doorStateFeedback = RDOBH_DOOR_FEEDBACK_OPENED;

    const bool handled = RearDoorOpenBlockHandler_HandleEvent(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_TRUE(output.openRequestBlock);
    EXPECT_EQ(RDOBH_DRIVER_NOTICE_BLOCK_FAILURE, output.driverNotice);
    EXPECT_EQ(RDOBH_RESULT_BLOCK_FAILED, output.blockResult);
}

/* =========================================================================
 * 4. Invalid CL state → safe block
 * ========================================================================= */

TEST_F(RearDoorOpenBlockHandlerTest, InvalidChildLockStateFallsBackToSafeBlock)
{
    std::array<unsigned char, sizeof(ChildLockState_t)> invalidRawStateBytes;
    invalidRawStateBytes.fill(0xFFU);

    input.rearDoorInnerHandleEvent = true;
    std::memcpy(&input.currentClState, invalidRawStateBytes.data(), sizeof(input.currentClState));
    input.doorStateFeedback = RDOBH_DOOR_FEEDBACK_UNKNOWN;

    const bool handled = RearDoorOpenBlockHandler_HandleEvent(&input, &output);

    ASSERT_TRUE(handled);
    EXPECT_TRUE(output.openRequestBlock);
    EXPECT_EQ(RDOBH_DRIVER_NOTICE_BLOCK_FAILURE, output.driverNotice);
    EXPECT_EQ(RDOBH_RESULT_SAFE_BLOCKED, output.blockResult);
}

/* =========================================================================
 * 5. NULL pointer guards
 * ========================================================================= */

TEST_F(RearDoorOpenBlockHandlerTest, NullInputIsRejected)
{
    const bool handled = RearDoorOpenBlockHandler_HandleEvent(nullptr, &output);

    EXPECT_FALSE(handled);
}

TEST_F(RearDoorOpenBlockHandlerTest, NullOutputIsRejected)
{
    const bool handled = RearDoorOpenBlockHandler_HandleEvent(&input, nullptr);

    EXPECT_FALSE(handled);
}
