/**
 * @file test_state_persistence_manager.cpp
 * @brief Failing Google Tests for F-05 StatePersistenceManager.
 *
 * @details These tests define the expected persistence and restore behavior for
 *          IGN OFF, IGN ON, and reset handling. The current C implementation is
 *          only a defensive stub, so several tests are intentionally red until
 *          F-05 behavior is completed.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */

#include <gtest/gtest.h>

extern "C" {
#include "childlock/state_persistence_manager.h"
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

TEST_F(StatePersistenceManagerTest, PersistChildLockStateOnIgnitionOff)
{
    const bool handled =
        StatePersistenceManager_HandleIgnitionOff(&manager, CL_STATE_ON, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.writeCallCount);
    EXPECT_EQ(CL_STATE_ON, g_backend.storedState);
    EXPECT_TRUE(result.persistSucceeded);
    EXPECT_EQ(CL_STATE_ON, result.savedClState);
}

TEST_F(StatePersistenceManagerTest, RestorePersistedStateOnIgnitionOn)
{
    g_backend.storedState = CL_STATE_ON;

    const bool handled = StatePersistenceManager_HandleIgnitionOn(&manager, &result);

    ASSERT_TRUE(handled);
    EXPECT_EQ(1U, g_backend.readCallCount);
    EXPECT_EQ(SPM_RESTORE_STATUS_RESTORED, result.restoreStatus);
    EXPECT_EQ(CL_STATE_ON, result.restoredClState);
    EXPECT_TRUE(result.syncRequested);
}

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

    const bool handled = StatePersistenceManager_HandleReset(&manager, &result);

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
