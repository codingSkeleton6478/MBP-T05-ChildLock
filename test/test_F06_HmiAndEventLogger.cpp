/**
 * @file    test_F06_HmiAndEventLogger.cpp
 * @brief   Unit tests for F-06: HMI and Event Logger Module
 *
 * @details Tests the initialization and event distribution to injected callbacks.
 *
 * @version 1.1.0
 * @date    2026-03-14
 * @author  AI Model: Gemini
 * @copyright Synetics 20 CopyrightⓒSynetics_
 */

#include <gtest/gtest.h>

extern "C" {
#include "F06_HmiAndEventLogger.h"
}

/* 팀의 Mock 스타일 컨벤션 적용 */
namespace
{
struct FakeLoggerBackend
{
    WarningMsgId_t lastMsgId;
    bool lastSoundReq;
    F06_EventType_t lastEvtType;
    ChildLockState_t lastCLState;
    FaultFlag_t lastFault;
    
    unsigned int displayCallCount;
    unsigned int logCallCount;
    unsigned int dtcCallCount;
};

FakeLoggerBackend g_backend;

void FakeDisplayCb(WarningMsgId_t msgId, bool soundReq)
{
    g_backend.displayCallCount++;
    g_backend.lastMsgId = msgId;
    g_backend.lastSoundReq = soundReq;
}

void FakeLogCb(F06_EventType_t type, ChildLockState_t state)
{
    g_backend.logCallCount++;
    g_backend.lastEvtType = type;
    g_backend.lastCLState = state;
}

void FakeDtcStoreCb(FaultFlag_t fault)
{
    g_backend.dtcCallCount++;
    g_backend.lastFault = fault;
}
} // namespace

class F06HmiLoggerTest : public ::testing::Test
{
protected:
    F06_HmiLoggerContext_t ctx;
    F06_HmiLoggerConfig_t config;

    void SetUp() override
    {
        // Reset Backend
        g_backend.lastMsgId = WARNING_MSG_NONE;
        g_backend.lastSoundReq = false;
        g_backend.lastEvtType = F06_EVENT_TYPE_NONE;
        g_backend.lastCLState = CL_STATE_OFF;
        g_backend.lastFault = FAULT_FLAG_NONE;
        
        g_backend.displayCallCount = 0U;
        g_backend.logCallCount = 0U;
        g_backend.dtcCallCount = 0U;

        // Init Config
        config.displayCb = FakeDisplayCb;
        config.logCb = FakeLogCb;
        config.dtcCb = FakeDtcStoreCb;

        ASSERT_TRUE(F06_HmiAndEventLogger_Init(&ctx, &config));
    }
};

/* =========================================================================
 * 1. Initialization Tests
 * ========================================================================= */

TEST_F(F06HmiLoggerTest, Initialization_Success)
{
    EXPECT_TRUE(ctx.isInitialized);
}

TEST_F(F06HmiLoggerTest, Initialization_FailWithMissingCallback)
{
    F06_HmiLoggerContext_t failCtx;
    F06_HmiLoggerConfig_t failConfig = config;
    failConfig.logCb = NULL; // Missing one callback

    const bool success = F06_HmiAndEventLogger_Init(&failCtx, &failConfig);

    EXPECT_FALSE(success);
    EXPECT_FALSE(failCtx.isInitialized);
}

/* =========================================================================
 * 2. Process Positive Scenarios
 * ========================================================================= */

TEST_F(F06HmiLoggerTest, Positive_ProcessValidRequests)
{
    // Simulate a fault event that triggers HMI, NVM Log, and DTC
    F06_HmiAndEventLogger_Process(&ctx, 
                                  WARNING_MSG_SENSOR_FAULT, 
                                  true, 
                                  F06_EVENT_TYPE_ERROR, 
                                  FAULT_FLAG_REAR_SENSOR, 
                                  CL_STATE_ON);

    EXPECT_EQ(1U, g_backend.displayCallCount);
    EXPECT_EQ(WARNING_MSG_SENSOR_FAULT, g_backend.lastMsgId);
    EXPECT_TRUE(g_backend.lastSoundReq);

    EXPECT_EQ(1U, g_backend.logCallCount);
    EXPECT_EQ(F06_EVENT_TYPE_ERROR, g_backend.lastEvtType);
    EXPECT_EQ(CL_STATE_ON, g_backend.lastCLState);

    EXPECT_EQ(1U, g_backend.dtcCallCount);
    EXPECT_EQ(FAULT_FLAG_REAR_SENSOR, g_backend.lastFault);
}

TEST_F(F06HmiLoggerTest, Positive_ProcessPartialRequests)
{
    // Only request a log, no HMI, no DTC
    F06_HmiAndEventLogger_Process(&ctx, 
                                  WARNING_MSG_NONE, 
                                  false, 
                                  F06_EVENT_TYPE_STATE_CHANGE, 
                                  FAULT_FLAG_NONE, 
                                  CL_STATE_ON);

    EXPECT_EQ(0U, g_backend.displayCallCount);
    EXPECT_EQ(1U, g_backend.logCallCount);
    EXPECT_EQ(0U, g_backend.dtcCallCount);
}

/* =========================================================================
 * 3. Robustness Scenarios
 * ========================================================================= */

TEST_F(F06HmiLoggerTest, Robustness_ProcessUninitializedOrNull)
{
    // Uninitialized context
    ctx.isInitialized = false;
    F06_HmiAndEventLogger_Process(&ctx, WARNING_MSG_CL_ALREADY_ON, true, F06_EVENT_TYPE_ERROR, FAULT_FLAG_HMI_FAULT, CL_STATE_OFF);
    
    EXPECT_EQ(0U, g_backend.displayCallCount); // Should not be called

    // Null pointer
    F06_HmiAndEventLogger_Process(NULL, WARNING_MSG_CL_ALREADY_ON, true, F06_EVENT_TYPE_ERROR, FAULT_FLAG_HMI_FAULT, CL_STATE_OFF);

    EXPECT_EQ(0U, g_backend.displayCallCount);
}

/* =========================================================================
 * 4. Init failure variants
 * ========================================================================= */

TEST_F(F06HmiLoggerTest, Init_NullCtx_ReturnsFalse)
{
    const bool success = F06_HmiAndEventLogger_Init(nullptr, &config);
    EXPECT_FALSE(success);
}

TEST_F(F06HmiLoggerTest, Init_NullConfig_ReturnsFalse)
{
    F06_HmiLoggerContext_t c;
    const bool success = F06_HmiAndEventLogger_Init(&c, nullptr);
    EXPECT_FALSE(success);
    EXPECT_FALSE(c.isInitialized);
}

TEST_F(F06HmiLoggerTest, Init_NullDisplayCb_ReturnsFalse)
{
    F06_HmiLoggerContext_t c;
    F06_HmiLoggerConfig_t badConfig = config;
    badConfig.displayCb = nullptr;

    EXPECT_FALSE(F06_HmiAndEventLogger_Init(&c, &badConfig));
    EXPECT_FALSE(c.isInitialized);
}

TEST_F(F06HmiLoggerTest, Init_NullDtcCb_ReturnsFalse)
{
    F06_HmiLoggerContext_t c;
    F06_HmiLoggerConfig_t badConfig = config;
    badConfig.dtcCb = nullptr;

    EXPECT_FALSE(F06_HmiAndEventLogger_Init(&c, &badConfig));
    EXPECT_FALSE(c.isInitialized);
}

/* =========================================================================
 * 5. Process edge cases
 * ========================================================================= */

TEST_F(F06HmiLoggerTest, Process_AllNone_NoCallbacksCalled)
{
    F06_HmiAndEventLogger_Process(&ctx,
                                  WARNING_MSG_NONE,
                                  false,
                                  F06_EVENT_TYPE_NONE,
                                  FAULT_FLAG_NONE,
                                  CL_STATE_OFF);

    EXPECT_EQ(0U, g_backend.displayCallCount);
    EXPECT_EQ(0U, g_backend.logCallCount);
    EXPECT_EQ(0U, g_backend.dtcCallCount);
}

TEST_F(F06HmiLoggerTest, Process_OnlyFaultSet_OnlyDtcCalled)
{
    F06_HmiAndEventLogger_Process(&ctx,
                                  WARNING_MSG_NONE,
                                  false,
                                  F06_EVENT_TYPE_NONE,
                                  FAULT_FLAG_SPEED_FAULT,
                                  CL_STATE_OFF);

    EXPECT_EQ(0U, g_backend.displayCallCount);
    EXPECT_EQ(0U, g_backend.logCallCount);
    EXPECT_EQ(1U, g_backend.dtcCallCount);
    EXPECT_EQ(FAULT_FLAG_SPEED_FAULT, g_backend.lastFault);
}