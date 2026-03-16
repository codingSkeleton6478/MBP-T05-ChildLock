/**
 * @file F05_StatePersistenceManager.h
 * @brief Public API for F-05 StatePersistenceManager.
 *
 * @details This module provides a small, testable interface for persisting the
 *          child lock state before power-down and restoring it after IGN ON or
 *          reset. Storage access and Door ECU re-synchronization are injected
 *          through callbacks so the module remains deterministic and avoids
 *          dynamic allocation.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */

#ifndef F05_STATE_PERSISTENCE_MANAGER_H
#define F05_STATE_PERSISTENCE_MANAGER_H

#include <stdbool.h>

#include "childlock_types.h"

/**
 * @brief Restore result reported by F-05.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
typedef enum
{
    SPM_RESTORE_STATUS_NOT_REQUESTED = 0U, /**< No restore was requested. */
    SPM_RESTORE_STATUS_RESTORED      = 1U, /**< Valid state restored. */
    SPM_RESTORE_STATUS_UNAVAILABLE   = 2U, /**< No persisted state available. */
    SPM_RESTORE_STATUS_INVALID       = 3U, /**< Persisted data invalid. */
    SPM_RESTORE_STATUS_ERROR         = 4U  /**< Dependency or execution error. */
} StatePersistenceManager_RestoreStatus_t;

/**
 * @brief Persist callback used to store the current child lock state.
 *
 * @param[in] state Child lock state to persist.
 * @return true if the state was stored successfully, otherwise false.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
typedef bool (*StatePersistenceManager_WriteCallback_t)(ChildLockState_t state);

/**
 * @brief Restore callback used to retrieve a persisted child lock state.
 *
 * @param[out] state Pointer receiving the restored state when available.
 * @param[out] isValid Pointer receiving persisted data validity information.
 * @return true if persisted data was available to inspect, otherwise false.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
typedef bool (*StatePersistenceManager_ReadCallback_t)(ChildLockState_t *state,
                                                       bool *isValid);

/**
 * @brief Callback used to request Door ECU re-synchronization after restore.
 *
 * @param[in] restoredState Restored child lock state to re-synchronize.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
typedef void (*StatePersistenceManager_SyncCallback_t)(ChildLockState_t restoredState);

/**
 * @brief Configuration of injected dependencies for F-05.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
typedef struct
{
    StatePersistenceManager_WriteCallback_t writePersistedState; /**< NVM write hook. */
    StatePersistenceManager_ReadCallback_t readPersistedState;   /**< NVM read hook. */
    StatePersistenceManager_SyncCallback_t requestDoorEcuSync;   /**< Door ECU sync hook. */
} StatePersistenceManager_Config_t;

/**
 * @brief Output data produced by persist/restore operations.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
typedef struct
{
    ChildLockState_t savedClState; /**< Last state requested for persistence. */
    ChildLockState_t restoredClState; /**< Restored state or safe default. */
    StatePersistenceManager_RestoreStatus_t restoreStatus; /**< Restore outcome. */
    bool persistSucceeded; /**< true when persistence completed successfully. */
    bool syncRequested; /**< true when Door ECU re-synchronization was requested. */
} StatePersistenceManager_Result_t;

/**
 * @brief Runtime context for F-05.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
typedef struct
{
    StatePersistenceManager_Config_t config; /**< Injected dependencies. */
    bool isInitialized; /**< Defensive initialization guard. */
} StatePersistenceManager_t;

/**
 * @brief Initializes the F-05 context with dependency callbacks.
 *
 * @param[out] manager Pointer to the runtime context.
 * @param[in] config Pointer to the dependency configuration.
 * @return true if initialization succeeded, otherwise false.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
bool StatePersistenceManager_Init(StatePersistenceManager_t *manager,
                                  const StatePersistenceManager_Config_t *config);

/**
 * @brief Persists the current child lock state during IGN OFF handling.
 *
 * @param[in] manager Pointer to the initialized runtime context.
 * @param[in] currentClState Current child lock state to persist.
 * @param[out] result Pointer receiving the operation result.
 * @return true if the request was processed, otherwise false.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
bool StatePersistenceManager_HandleIgnitionOff(StatePersistenceManager_t *manager,
                                               ChildLockState_t currentClState,
                                               StatePersistenceManager_Result_t *result);

/**
 * @brief Restores the persisted child lock state during IGN ON handling.
 *
 * @param[in] manager Pointer to the initialized runtime context.
 * @param[out] result Pointer receiving the restore result.
 * @return true if the request was processed, otherwise false.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
bool StatePersistenceManager_HandleIgnitionOn(StatePersistenceManager_t *manager,
                                              StatePersistenceManager_Result_t *result);

/**
 * @brief Restores the persisted child lock state after reset handling.
 *
 * @param[in] manager Pointer to the initialized runtime context.
 * @param[out] result Pointer receiving the restore result.
 * @return true if the request was processed, otherwise false.
 *
 * @traceability SDD-F-05
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-3, UC-7
 */
bool StatePersistenceManager_HandleReset(StatePersistenceManager_t *manager,
                                         StatePersistenceManager_Result_t *result);

#endif /* F05_STATE_PERSISTENCE_MANAGER_H */
