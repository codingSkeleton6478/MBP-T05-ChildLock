/**
 * @file test_F05_StatePersistenceManager.cpp
 * @brief Unit tests for F-05 StatePersistenceManager.
 *
 * @details Tests define expected persistence and restore behavior for
 *          IGN OFF, IGN ON, and reset handling. Includes full NULL/
 *          uninitialized guard coverage and failed-write scenarios.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */

#include <gtest/gtest.h>

extern "C" {
#include "F05_StatePersistenceManager.h"
}

namespace
{
struct FakePersistenceBackend
{
    bool writeReturnValue;
    bool readReturnValue;
    bool readValidValue;
    ChildLockState_t storedState;
    ChildLockState_t lastSyncedState;
    unsigned int writeCallCount;
    unsigned int readCallCount;
    unsigned int syncCallCount;
};

FakePersistenceBackend g_backend;

bool FakeWritePersistedState(ChildLockState_t state)
{
    g_backend.writeCallCount++;
    g_backend.storedState = state;
    return g_backend.writeReturnValue;
}

bool FakeReadPersistedState(ChildLockState_t *state, bool *isValid)
{
    g_backend.readCallCount++;

    if ((state != nullptr) && (isValid != nullptr))
    {
        *state = g_backend.storedState;
        *isValid = g_backend.readValidValue;
    }

    return g_backend.readReturnValue;
}

void FakeRequestDoorEcuSync(ChildLockState_t restoredState)
{
    g_backend.syncCallCount++;
    g_backend.lastSyncedState = restoredState;
}
}  // namespace

class StatePersistenceManagerTest : public ::testing::Test
{
protected:
    StatePersistenceManager_t manager;
    StatePersistenceManager_Config_t config;
    StatePersistenceManager_Result_t result;

    void SetUp() override
    {
        g_backend.writeReturnValue = true;
        g_backend.readReturnValue = true;
        g_backend.readValidValue = true;
        g_backend.storedState = CL_STATE_OFF;
        g_backend.lastSyncedState = CL_STATE_OFF;
        g_backend.writeCallCount = 0U;
        g_backend.readCallCount = 0U;
        g_backend.syncCallCount = 0U;

        config.writePersistedState = FakeWritePersistedState;
        config.readPersistedState = FakeReadPersistedState;
        config.requestDoorEcuSync = FakeRequestDoorEcuSync;

        ASSERT_TRUE(StatePersistenceManager_Init(&manager, &config));
    }
};

/* =========================================================================
 * 1. Initialization tests
 * ========================================================================= */

TEST_F(StatePersistenceManagerTest, Init_NullManager_ReturnsFalse)
{
    EXPECT_FALSE(StatePersistenceManager_Init(nullptr, &config));
}

TEST_F(StatePersistenceManagerTest, Init_NullConfig_ReturnsFalse)
{
    StatePersistenceManager_t m;
    EXPECT_FALSE(StatePersistenceManager_Init(&m, nullptr));
    EXPECT_FALSE(m.isInitialized);
}

TEST_F(StatePersistenceManagerTest, Init_MissingWriteCallback_ReturnsFalse)
{
    StatePersistenceManager_t m;
    StatePersistenceManager_Config_t c = config;
    c.writePersistedState = nullptr;

    EXPECT_FALSE(StatePersistenceManager_Init(&m, &c));
    EXPECT_FALSE(m.isInitialized);
}

TEST_F(StatePersistenceManagerTest, Init_MissingReadCallback_ReturnsFalse)
{
    StatePersistenceManager_t m;
    StatePersistenceManager_Config_t c = config;
    c.readPersistedState = nullptr;

    EXPECT_FALSE(StatePersistenceManager_Init(&m, &c));
    EXPECT_FALSE(m.isInitialized);
}

TEST_F(StatePersistenceManagerTest, Init_MissingSyncCallback_ReturnsFalse)
{
    StatePersistenceManager_t m;
    StatePersistenceManager_Config_t c = config;
    c.requestDoorEcuSync = nullptr;

    EXPECT_FALSE(StatePersistenceManager_Init(&m, &c));
    EXPECT_FALSE(m.isInitialized);
}

/* =========================================================================
 * 2. HandleIgnitionOff tests
 * ========================================================================= */

TEST_F(StatePersistenceManagerTest, PersistChildLockStateOnIgnitionOff_On)
{
    const bool handled =
        StatePersistenceManager_HandleIgnitionOff(&manager, CL_STATE_ON, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.writeCallCount);
    EXPECT_EQ(CL_STATE_ON, g_backend.storedState);
    EXPECT_TRUE(result.persistSucceeded);
    EXPECT_EQ(CL_STATE_ON, result.savedClState);
}

TEST_F(StatePersistenceManagerTest, PersistChildLockStateOnIgnitionOff_Off)
{
    const bool handled =
        StatePersistenceManager_HandleIgnitionOff(&manager, CL_STATE_OFF, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.writeCallCount);
    EXPECT_EQ(CL_STATE_OFF, g_backend.storedState);
    EXPECT_TRUE(result.persistSucceeded);
    EXPECT_EQ(CL_STATE_OFF, result.savedClState);
}

TEST_F(StatePersistenceManagerTest, HandleIgnitionOff_WriteFails_PersistFailed)
{
    g_backend.writeReturnValue = false;

    const bool handled =
        StatePersistenceManager_HandleIgnitionOff(&manager, CL_STATE_ON, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.writeCallCount);
    EXPECT_FALSE(result.persistSucceeded);
}

TEST_F(StatePersistenceManagerTest, HandleIgnitionOff_NullManager_ReturnsFalse)
{
    const bool handled =
        StatePersistenceManager_HandleIgnitionOff(nullptr, CL_STATE_ON, &result);

    EXPECT_FALSE(handled);
    EXPECT_EQ(0U, g_backend.writeCallCount);
}

TEST_F(StatePersistenceManagerTest, HandleIgnitionOff_NullResult_ReturnsFalse)
{
    const bool handled =
        StatePersistenceManager_HandleIgnitionOff(&manager, CL_STATE_ON, nullptr);

    EXPECT_FALSE(handled);
    EXPECT_EQ(0U, g_backend.writeCallCount);
}

TEST_F(StatePersistenceManagerTest, HandleIgnitionOff_UninitManager_ReturnsFalse)
{
    manager.isInitialized = false;

    const bool handled =
        StatePersistenceManager_HandleIgnitionOff(&manager, CL_STATE_ON, &result);

    EXPECT_FALSE(handled);
    EXPECT_EQ(0U, g_backend.writeCallCount);
}

/* =========================================================================
 * 3. HandleIgnitionOn tests
 * ========================================================================= */

TEST_F(StatePersistenceManagerTest, RestorePersistedStateOnIgnitionOn_ClOn)
{
    g_backend.storedState = CL_STATE_ON;

    const bool handled = StatePersistenceManager_HandleIgnitionOn(&manager, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.readCallCount);
    EXPECT_EQ(SPM_RESTORE_STATUS_RESTORED, result.restoreStatus);
    EXPECT_EQ(CL_STATE_ON, result.restoredClState);
    EXPECT_TRUE(result.syncRequested);
}

TEST_F(StatePersistenceManagerTest, RestorePersistedStateOnIgnitionOn_ClOff)
{
    g_backend.storedState = CL_STATE_OFF;

    const bool handled = StatePersistenceManager_HandleIgnitionOn(&manager, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(SPM_RESTORE_STATUS_RESTORED, result.restoreStatus);
    EXPECT_EQ(CL_STATE_OFF, result.restoredClState);
    EXPECT_TRUE(result.syncRequested);
    EXPECT_EQ(1U, g_backend.syncCallCount);
    EXPECT_EQ(CL_STATE_OFF, g_backend.lastSyncedState);
}

TEST_F(StatePersistenceManagerTest, HandleUnavailablePersistedStateSafely)
{
    g_backend.readReturnValue = false;

    const bool handled = StatePersistenceManager_HandleIgnitionOn(&manager, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(SPM_RESTORE_STATUS_UNAVAILABLE, result.restoreStatus);
    EXPECT_EQ(CL_STATE_OFF, result.restoredClState);
    EXPECT_FALSE(result.syncRequested);
    EXPECT_EQ(0U, g_backend.syncCallCount);
}

TEST_F(StatePersistenceManagerTest, HandleInvalidPersistedStateSafely)
{
    g_backend.readValidValue = false;
    g_backend.storedState = CL_STATE_ON;

    const bool handled = StatePersistenceManager_HandleIgnitionOn(&manager, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(SPM_RESTORE_STATUS_INVALID, result.restoreStatus);
    EXPECT_EQ(CL_STATE_OFF, result.restoredClState);
    EXPECT_FALSE(result.syncRequested);
    EXPECT_EQ(0U, g_backend.syncCallCount);
}

TEST_F(StatePersistenceManagerTest, RequestResynchronizationWithDoorEcuAfterRestore)
{
    g_backend.storedState = CL_STATE_ON;

    const bool handled = StatePersistenceManager_HandleIgnitionOn(&manager, &result);

    ASSERT_TRUE(handled);
    EXPECT_TRUE(result.syncRequested);
    EXPECT_EQ(1U, g_backend.syncCallCount);
    EXPECT_EQ(CL_STATE_ON, g_backend.lastSyncedState);
}

TEST_F(StatePersistenceManagerTest, HandleIgnitionOn_NullManager_ReturnsFalse)
{
    const bool handled = StatePersistenceManager_HandleIgnitionOn(nullptr, &result);

    EXPECT_FALSE(handled);
    EXPECT_EQ(0U, g_backend.readCallCount);
}

TEST_F(StatePersistenceManagerTest, HandleIgnitionOn_NullResult_ReturnsFalse)
{
    const bool handled = StatePersistenceManager_HandleIgnitionOn(&manager, nullptr);

    EXPECT_FALSE(handled);
    EXPECT_EQ(0U, g_backend.readCallCount);
}

TEST_F(StatePersistenceManagerTest, HandleIgnitionOn_UninitManager_ReturnsFalse)
{
    manager.isInitialized = false;

    const bool handled = StatePersistenceManager_HandleIgnitionOn(&manager, &result);

    EXPECT_FALSE(handled);
    EXPECT_EQ(0U, g_backend.readCallCount);
}

/* =========================================================================
 * 4. HandleReset tests
 * ========================================================================= */

TEST_F(StatePersistenceManagerTest, RestorePersistedStateOnReset)
{
    g_backend.storedState = CL_STATE_ON;

    const bool handled = StatePersistenceManager_HandleReset(&manager, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.readCallCount);
    EXPECT_EQ(SPM_RESTORE_STATUS_RESTORED, result.restoreStatus);
    EXPECT_EQ(CL_STATE_ON, result.restoredClState);
    EXPECT_TRUE(result.syncRequested);
}

TEST_F(StatePersistenceManagerTest, HandleInvalidStateOnReset_StatusInvalid)
{
    g_backend.readValidValue = false;
    g_backend.storedState = CL_STATE_ON;

    const bool handled = StatePersistenceManager_HandleReset(&manager, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(SPM_RESTORE_STATUS_INVALID, result.restoreStatus);
    EXPECT_FALSE(result.syncRequested);
}

TEST_F(StatePersistenceManagerTest, HandleReset_NullManager_ReturnsFalse)
{
    const bool handled = StatePersistenceManager_HandleReset(nullptr, &result);

    EXPECT_FALSE(handled);
}
