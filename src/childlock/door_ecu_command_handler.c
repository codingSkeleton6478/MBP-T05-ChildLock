/**
 * @file door_ecu_command_handler.c
 * @brief Minimal C11 implementation for F-03 DoorEcuCommandHandler.
 *
 * @details This implementation issues a child lock command based on the
 *          target state, evaluates Door ECU ACK reception, and applies retry
 *          exhaustion + DTC reporting when ACK is not received.
 *
 * @traceability SDD-F-03
 * @req_id REQ-SW-CL-003
 * @asil ASIL-TBD
 * @note Related UC: UC-1, UC-2, UC-3, UC-5
 */

#include "childlock/door_ecu_command_handler.h"

#include <stddef.h>

/**
 * @brief Resets F-03 output to safe defaults.
 *
 * @param[out] output Pointer to output structure.
 */
static void DoorEcuCommandHandler_ClearOutput(DoorEcuCommandHandler_Output_t *output)
{
    if (output != NULL)
    {
        output->childLockCommand = DECH_COMMAND_NONE;
        output->ackStatus = DECH_ACK_STATUS_NOT_CHECKED;
        output->retryStatus = DECH_RETRY_STATUS_NOT_REQUIRED;
        output->dtc = FAULT_FLAG_NONE;
        output->retryCount = 0U;
    }
}

/**
 * @brief Validates child lock target state.
 *
 * @param[in] state Target state to validate.
 * @return true when target state is supported.
 */
static bool DoorEcuCommandHandler_IsValidState(ChildLockState_t state)
{
    return ((state == CL_STATE_OFF) || (state == CL_STATE_ON));
}

/**
 * @brief Validates F-03 decision reason range.
 *
 * @param[in] reason Decision reason to validate.
 * @return true when reason enum value is supported.
 */
static bool DoorEcuCommandHandler_IsValidReason(
    DoorEcuCommandHandler_DecisionReason_t reason)
{
    return ((reason == DECH_REASON_DRIVER_REQUEST)
        || (reason == DECH_REASON_AUTO_LOCK)
        || (reason == DECH_REASON_EMERGENCY_RELEASE)
        || (reason == DECH_REASON_SAFE_LOCKED)
        || (reason == DECH_REASON_STATE_RESTORE));
}

/**
 * @brief Maps child lock state to Door ECU command.
 *
 * @param[in] state Target child lock state.
 * @return Corresponding command for Door ECU.
 */
static DoorEcuCommandHandler_ChildLockCommand_t
DoorEcuCommandHandler_MapCommand(ChildLockState_t state)
{
    DoorEcuCommandHandler_ChildLockCommand_t command = DECH_COMMAND_CL_ON;

    if (state == CL_STATE_OFF)
    {
        command = DECH_COMMAND_CL_OFF;
    }

    return command;
}

/**
 * @brief Handles Door ECU command, ACK check, and retry policy for F-03.
 *
 * @details If ACK is received, retry remains unnecessary and no DTC is set.
 *          If ACK is not received, retry is considered exhausted according to
 *          `MAX_ECU_RETRY_COUNT` and `FAULT_FLAG_ECU_ACK_FAIL` is reported.
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
    DoorEcuCommandHandler_Output_t *output)
{
    bool isHandled = false;

    DoorEcuCommandHandler_ClearOutput(output);

    if ((input != NULL) && (output != NULL)
        && (DoorEcuCommandHandler_IsValidState(input->targetClState) == true)
        && (DoorEcuCommandHandler_IsValidReason(input->decisionReason) == true))
    {
        output->childLockCommand = DoorEcuCommandHandler_MapCommand(input->targetClState);
        isHandled = true;

        if (input->doorEcuAck == true)
        {
            output->ackStatus = DECH_ACK_STATUS_RECEIVED;
        }
        else
        {
            output->ackStatus = DECH_ACK_STATUS_NOT_RECEIVED;
            output->retryStatus = DECH_RETRY_STATUS_EXHAUSTED;
            output->dtc = FAULT_FLAG_ECU_ACK_FAIL;
            output->retryCount = MAX_ECU_RETRY_COUNT;
        }
    }

    return isHandled;
}
