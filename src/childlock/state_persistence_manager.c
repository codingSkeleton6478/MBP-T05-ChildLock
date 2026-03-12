/**
 * @file state_persistence_manager.c
 * @brief Stub implementation for F-05 StatePersistenceManager groundwork.
 *
 * @details This file intentionally implements only defensive scaffolding for
 *          the API so tests can be written first. The restore/persist behavior
 *          is not completed yet, which keeps the Google Tests red by design.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */

#include "childlock/state_persistence_manager.h"

#include <stddef.h>

/**
 * @brief Clears the public result structure to safe defaults.
 *
 * @param[out] result Pointer to the result object.
 */
static void StatePersistenceManager_ClearResult(StatePersistenceManager_Result_t *result)
{
    if (result != NULL)
    {
        result->savedClState = CL_STATE_OFF;
        result->restoredClState = CL_STATE_OFF;
        result->restoreStatus = SPM_RESTORE_STATUS_NOT_REQUESTED;
        result->persistSucceeded = false;
        result->syncRequested = false;
    }
}

bool StatePersistenceManager_Init(StatePersistenceManager_t *manager,
                                  const StatePersistenceManager_Config_t *config)
{
    bool isInitialized = false;

    if ((manager != NULL) && (config != NULL))
    {
        manager->config = *config;
        manager->isInitialized = true;
        isInitialized = true;
    }

    return isInitialized;
}

bool StatePersistenceManager_HandleIgnitionOff(StatePersistenceManager_t *manager,
                                               ChildLockState_t currentClState,
                                               StatePersistenceManager_Result_t *result)
{
    bool isHandled = false;

    StatePersistenceManager_ClearResult(result);

    if ((manager != NULL) && (result != NULL) && (manager->isInitialized == true))
    {
        result->savedClState = currentClState;
        /* TODO(SDD-F-05): Persist currentClState through writePersistedState. */
        isHandled = true;
    }

    return isHandled;
}

/**
 * @brief Shared stubbed restore path for IGN ON and reset.
 *
 * @param[in] manager Pointer to the runtime context.
 * @param[out] result Pointer to the result object.
 * @return true when the request is accepted for processing.
 */
static bool StatePersistenceManager_HandleRestore(StatePersistenceManager_t *manager,
                                                  StatePersistenceManager_Result_t *result)
{
    bool isHandled = false;

    StatePersistenceManager_ClearResult(result);

    if ((manager != NULL) && (result != NULL) && (manager->isInitialized == true))
    {
        /* TODO(SDD-F-05): Read persisted state and request Door ECU sync. */
        result->restoreStatus = SPM_RESTORE_STATUS_UNAVAILABLE;
        isHandled = true;
    }

    return isHandled;
}

bool StatePersistenceManager_HandleIgnitionOn(StatePersistenceManager_t *manager,
                                              StatePersistenceManager_Result_t *result)
{
    return StatePersistenceManager_HandleRestore(manager, result);
}

bool StatePersistenceManager_HandleReset(StatePersistenceManager_t *manager,
                                         StatePersistenceManager_Result_t *result)
{
    return StatePersistenceManager_HandleRestore(manager, result);
}
