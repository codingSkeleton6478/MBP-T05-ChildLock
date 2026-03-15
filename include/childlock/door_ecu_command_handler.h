/**
 * @file door_ecu_command_handler.h
 * @brief Public API for F-03 DoorEcuCommandHandler.
 *
 * @details This module translates a target child lock state decision into a
 *          Door ECU command, evaluates ACK reception, and applies the retry
 *          exhaustion policy when ACK is not received.
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */

#ifndef CHILDLOCK_DOOR_ECU_COMMAND_HANDLER_H
#define CHILDLOCK_DOOR_ECU_COMMAND_HANDLER_H

#include <stdbool.h>
#include <stdint.h>

#include "childlock_types.h"

/**
 * @brief Decision reason supplied from upstream state decision logic (F-02).
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */
typedef enum
{
    DECH_REASON_DRIVER_REQUEST = 0U,     /**< Driver switch-requested transition. */
    DECH_REASON_AUTO_LOCK = 1U,          /**< Auto-lock condition transition. */
    DECH_REASON_EMERGENCY_RELEASE = 2U,  /**< Crash-triggered emergency release. */
    DECH_REASON_SAFE_LOCKED = 3U,        /**< Safe-state lock enforcement. */
    DECH_REASON_STATE_RESTORE = 4U       /**< Restored state synchronization. */
} DoorEcuCommandHandler_DecisionReason_t;

/**
 * @brief Command sent to Door ECU by F-03.
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */
typedef enum
{
    DECH_COMMAND_NONE = 0U,    /**< No command issued. */
    DECH_COMMAND_CL_OFF = 1U,  /**< Request Child Lock OFF. */
    DECH_COMMAND_CL_ON = 2U    /**< Request Child Lock ON. */
} DoorEcuCommandHandler_ChildLockCommand_t;

/**
 * @brief ACK result classified by F-03.
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */
typedef enum
{
    DECH_ACK_STATUS_NOT_CHECKED = 0U,  /**< Command not processed. */
    DECH_ACK_STATUS_RECEIVED = 1U,     /**< ACK received from Door ECU. */
    DECH_ACK_STATUS_NOT_RECEIVED = 2U  /**< ACK not received. */
} DoorEcuCommandHandler_AckStatus_t;

/**
 * @brief Retry status classified by F-03.
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */
typedef enum
{
    DECH_RETRY_STATUS_NOT_REQUIRED = 0U,  /**< ACK succeeded without retries. */
    DECH_RETRY_STATUS_EXHAUSTED = 1U      /**< Retries exhausted with no ACK. */
} DoorEcuCommandHandler_RetryStatus_t;

/**
 * @brief Inputs provided to F-03 DoorEcuCommandHandler.
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */
typedef struct
{
    ChildLockState_t targetClState; /**< Target child lock state decided by F-02. */
    DoorEcuCommandHandler_DecisionReason_t decisionReason; /**< Decision reason from F-02. */
    bool doorEcuAck; /**< true when Door ECU ACK was received. */
} DoorEcuCommandHandler_Input_t;

/**
 * @brief Outputs produced by F-03 DoorEcuCommandHandler.
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */
typedef struct
{
    DoorEcuCommandHandler_ChildLockCommand_t childLockCommand; /**< Command issued to Door ECU. */
    DoorEcuCommandHandler_AckStatus_t ackStatus; /**< ACK handling result. */
    DoorEcuCommandHandler_RetryStatus_t retryStatus; /**< Retry policy result. */
    FaultFlag_t dtc; /**< DTC fault flag for logging/store. */
    uint8_t retryCount; /**< Number of retries attempted by policy. */
} DoorEcuCommandHandler_Output_t;

/**
 * @brief Handles Door ECU command, ACK check, and retry policy for F-03.
 *
 * @details When ACK is received, retry is not required and DTC remains clear.
 *          When ACK is not received, retry is considered exhausted up to the
 *          configured limit (`MAX_ECU_RETRY_COUNT`) and ECU ACK failure is
 *          reported via DTC for logging by F-06.
 *
 * @param[in] input Pointer to F-03 input signals.
 * @param[out] output Pointer receiving F-03 output classification.
 * @return true when processing completed, otherwise false.
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */
bool DoorEcuCommandHandler_HandleCommand(
    const DoorEcuCommandHandler_Input_t *input,
    DoorEcuCommandHandler_Output_t *output);

#endif /* CHILDLOCK_DOOR_ECU_COMMAND_HANDLER_H */
