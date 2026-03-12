/**
 * @file test_ignition_off_status_alert.cpp
 * @brief Failing Google Tests for F-10 IgnitionOffStatusAlert.
 *
 * @details These tests define the expected UC-7 behavior for IGN OFF summary
 *          alerts, fallback handling, and alert-once suppression. The current
 *          C implementation is intentionally incomplete, so the behavioral
 *          tests remain red until F-10 is implemented.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */

#include <gtest/gtest.h>

extern "C" {
#include "childlock/ignition_off_status_alert.h"
}

namespace
{
struct FakeIgnitionOffStatusBackend
{
    bool queryReturnValue;
    ChildLockState_t queriedState;
    unsigned int queryCallCount;
};

FakeIgnitionOffStatusBackend g_backend;

bool FakeQueryChildLockState(ChildLockState_t *state)
{
    g_backend.queryCallCount++;

    if (state != nullptr)
    {
        *state = g_backend.queriedState;
    }

    return g_backend.queryReturnValue;
}
}  // namespace

class IgnitionOffStatusAlertTest : public ::testing::Test
{
protected:
    IgnitionOffStatusAlert_t alert;
    IgnitionOffStatusAlert_Config_t config;
    IgnitionOffStatusAlert_Input_t input;
    IgnitionOffStatusAlert_Output_t output;

    void SetUp() override
    {
        g_backend.queryReturnValue = true;
        g_backend.queriedState = CL_STATE_OFF;
        g_backend.queryCallCount = 0U;

        config.queryChildLockState = FakeQueryChildLockState;

        input.ignitionOffEvent = true;
        input.activeHmiAlert = false;
        input.hmiHealthy = true;

        ASSERT_TRUE(IgnitionOffStatusAlert_Init(&alert, &config));
    }
};

TEST_F(IgnitionOffStatusAlertTest, IgnitionOffAndChildLockOnEmitsOneSummaryAlert)
{
    g_backend.queriedState = CL_STATE_ON;

    const bool handled = IgnitionOffStatusAlert_HandleEvent(&alert, &input, &output);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.queryCallCount);
    EXPECT_TRUE(output.alertIssued);
    EXPECT_EQ(IOA_STATUS_MESSAGE_CL_ON_SUMMARY, output.statusSummaryMessage);
    EXPECT_EQ(IOA_ALERT_PRIORITY_STATUS, output.alertPriority);
    EXPECT_TRUE(output.eventLog);
}

TEST_F(IgnitionOffStatusAlertTest, IgnitionOffAndChildLockOffDoesNotEmitAlert)
{
    g_backend.queriedState = CL_STATE_OFF;

    const bool handled = IgnitionOffStatusAlert_HandleEvent(&alert, &input, &output);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.queryCallCount);
    EXPECT_FALSE(output.alertIssued);
    EXPECT_EQ(IOA_STATUS_MESSAGE_NONE, output.statusSummaryMessage);
    EXPECT_EQ(IOA_ALERT_PRIORITY_NONE, output.alertPriority);
    EXPECT_FALSE(output.eventLog);
}

TEST_F(IgnitionOffStatusAlertTest, StateQueryFailureEmitsFallbackMessage)
{
    g_backend.queryReturnValue = false;

    const bool handled = IgnitionOffStatusAlert_HandleEvent(&alert, &input, &output);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.queryCallCount);
    EXPECT_TRUE(output.alertIssued);
    EXPECT_EQ(IOA_STATUS_MESSAGE_QUERY_FALLBACK, output.statusSummaryMessage);
    EXPECT_EQ(IOA_ALERT_PRIORITY_STATUS, output.alertPriority);
    EXPECT_TRUE(output.eventLog);
}

TEST_F(IgnitionOffStatusAlertTest, RepeatedIgnitionOffDoesNotDuplicateAlertWhenPolicyApplies)
{
    IgnitionOffStatusAlert_Output_t secondOutput;
    g_backend.queriedState = CL_STATE_ON;

    const bool firstHandled = IgnitionOffStatusAlert_HandleEvent(&alert, &input, &output);
    const bool secondHandled = IgnitionOffStatusAlert_HandleEvent(&alert, &input, &secondOutput);

    ASSERT_TRUE(firstHandled);
    ASSERT_TRUE(secondHandled);
    EXPECT_TRUE(output.alertIssued);
    EXPECT_EQ(IOA_STATUS_MESSAGE_CL_ON_SUMMARY, output.statusSummaryMessage);
    EXPECT_FALSE(secondOutput.alertIssued);
    EXPECT_EQ(IOA_STATUS_MESSAGE_NONE, secondOutput.statusSummaryMessage);
    EXPECT_EQ(1U, g_backend.queryCallCount);
}
