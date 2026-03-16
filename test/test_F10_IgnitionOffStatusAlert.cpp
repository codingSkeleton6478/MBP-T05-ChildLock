/**
 * @file test_F10_IgnitionOffStatusAlert.cpp
 * @brief Unit tests for F-10 IgnitionOffStatusAlert.
 *
 * @details Tests define expected UC-7 behavior for IGN OFF summary alerts,
 *          fallback handling, alert-once suppression, and NULL guard coverage.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */

#include <gtest/gtest.h>

extern "C" {
#include "F10_IgnitionOffStatusAlert.h"
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

/* =========================================================================
 * 1. Initialization tests
 * ========================================================================= */

TEST_F(IgnitionOffStatusAlertTest, Init_NullAlert_ReturnsFalse)
{
    EXPECT_FALSE(IgnitionOffStatusAlert_Init(nullptr, &config));
}

TEST_F(IgnitionOffStatusAlertTest, Init_NullConfig_ReturnsFalse)
{
    IgnitionOffStatusAlert_t a;
    EXPECT_FALSE(IgnitionOffStatusAlert_Init(&a, nullptr));
    EXPECT_FALSE(a.isInitialized);
}

TEST_F(IgnitionOffStatusAlertTest, Init_NullCallback_ReturnsFalse)
{
    IgnitionOffStatusAlert_t a;
    IgnitionOffStatusAlert_Config_t c;
    c.queryChildLockState = nullptr;

    EXPECT_FALSE(IgnitionOffStatusAlert_Init(&a, &c));
    EXPECT_FALSE(a.isInitialized);
}

TEST_F(IgnitionOffStatusAlertTest, Init_ValidConfig_Succeeds)
{
    EXPECT_TRUE(alert.isInitialized);
    EXPECT_FALSE(alert.alertAlreadyIssued);
}

/* =========================================================================
 * 2. Alert emission
 * ========================================================================= */

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

TEST_F(IgnitionOffStatusAlertTest, RepeatedIgnitionOffDoesNotDuplicateAlert)
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
    EXPECT_EQ(1U, g_backend.queryCallCount); /* Latch prevents second query */
}

/* =========================================================================
 * 3. No ignition-off event
 * ========================================================================= */

TEST_F(IgnitionOffStatusAlertTest, NoIgnitionOffEvent_NoQueryNoAlert)
{
    input.ignitionOffEvent = false;
    g_backend.queriedState = CL_STATE_ON;

    const bool handled = IgnitionOffStatusAlert_HandleEvent(&alert, &input, &output);

    ASSERT_TRUE(handled);
    EXPECT_EQ(0U, g_backend.queryCallCount); /* No query when no IGN OFF event */
    EXPECT_FALSE(output.alertIssued);
    EXPECT_EQ(IOA_STATUS_MESSAGE_NONE, output.statusSummaryMessage);
}

TEST_F(IgnitionOffStatusAlertTest, AlertAlreadyIssued_IgnOff_NoQueryNoAlert)
{
    /* Simulate pre-latched state */
    alert.alertAlreadyIssued = true;
    g_backend.queriedState = CL_STATE_ON;

    const bool handled = IgnitionOffStatusAlert_HandleEvent(&alert, &input, &output);

    ASSERT_TRUE(handled);
    EXPECT_EQ(0U, g_backend.queryCallCount);
    EXPECT_FALSE(output.alertIssued);
}

/* =========================================================================
 * 4. NULL pointer guards
 * ========================================================================= */

TEST_F(IgnitionOffStatusAlertTest, NullAlert_ReturnsFalse)
{
    const bool handled = IgnitionOffStatusAlert_HandleEvent(nullptr, &input, &output);

    EXPECT_FALSE(handled);
}

TEST_F(IgnitionOffStatusAlertTest, NullInput_ReturnsFalse)
{
    const bool handled = IgnitionOffStatusAlert_HandleEvent(&alert, nullptr, &output);

    EXPECT_FALSE(handled);
}

TEST_F(IgnitionOffStatusAlertTest, NullOutput_ReturnsFalse)
{
    const bool handled = IgnitionOffStatusAlert_HandleEvent(&alert, &input, nullptr);

    EXPECT_FALSE(handled);
}

TEST_F(IgnitionOffStatusAlertTest, UninitAlert_ReturnsFalse)
{
    alert.isInitialized = false;

    const bool handled = IgnitionOffStatusAlert_HandleEvent(&alert, &input, &output);

    EXPECT_FALSE(handled);
}
