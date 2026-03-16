/**
 * @file F04_RearDoorOpenBlockHandler.c
 * @brief Minimal C11 implementation for F-04 RearDoorOpenBlockHandler.
 *
 * @details The implementation applies UC-4 blocking behavior using only the
 *          provided input signals. No dynamic memory is used. Invalid child
 *          lock states are treated conservatively as fail-safe blocked outputs.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */

#include "F04_RearDoorOpenBlockHandler.h"

#include <stddef.h>

/**
 * @brief Resets F-04 output to safe defaults.
 *
 * @param[out] output Pointer to output structure.
 */
static void RearDoorOpenBlockHandler_ClearOutput(RearDoorOpenBlockHandler_Output_t *output)
{
    if (output != NULL)
    {
        output->openRequestBlock = false;
        output->driverNotice = RDOBH_DRIVER_NOTICE_NONE;
        output->blockResult = RDOBH_RESULT_NO_EVENT;
    }
}

/**
 * @brief Validates child lock state values accepted by F-04.
 *
 * @param[in] state Child lock state value to validate.
 * @return true when state is a supported child lock enum value.
 */
static bool RearDoorOpenBlockHandler_IsValidState(ChildLockState_t state)
{
    return ((state == CL_STATE_OFF) || (state == CL_STATE_ON));
}

/**
 * @brief Handles UC-4 rear-door inner-handle open-block logic.
 *
 * @details The handler always clears output first. If no event exists, the
 *          default "no action" output is retained. A CL OFF event is allowed.
 *          A CL ON event is blocked, and door feedback is checked for mismatch.
 *          Invalid CL state values are handled with fail-safe block behavior.
 *
 * @param[in] input Pointer to current input signals.
 * @param[out] output Pointer receiving F-04 outputs.
 * @return true when processing completed, otherwise false.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */
// cppcheck-suppress unusedFunction
bool RearDoorOpenBlockHandler_HandleEvent(
    const RearDoorOpenBlockHandler_Input_t *input,
    RearDoorOpenBlockHandler_Output_t *output)
{
    bool isHandled = false;

    RearDoorOpenBlockHandler_ClearOutput(output);

    if ((input != NULL) && (output != NULL))
    {
        isHandled = true;

        if (input->rearDoorInnerHandleEvent == true)
        {
            if (RearDoorOpenBlockHandler_IsValidState(input->currentClState) == false)
            {
                output->openRequestBlock = true;
                output->driverNotice = RDOBH_DRIVER_NOTICE_BLOCK_FAILURE;
                output->blockResult = RDOBH_RESULT_SAFE_BLOCKED;
            }
            else if (input->currentClState == CL_STATE_OFF)
            {
                output->openRequestBlock = false;
                output->driverNotice = RDOBH_DRIVER_NOTICE_NONE;
                output->blockResult = RDOBH_RESULT_ALLOWED;
            }
            else
            {
                output->openRequestBlock = true;
                output->driverNotice = RDOBH_DRIVER_NOTICE_BLOCKED;
                output->blockResult = RDOBH_RESULT_BLOCKED;

                if (input->doorStateFeedback == RDOBH_DOOR_FEEDBACK_OPENED)
                {
                    output->driverNotice = RDOBH_DRIVER_NOTICE_BLOCK_FAILURE;
                    output->blockResult = RDOBH_RESULT_BLOCK_FAILED;
                }
                else
                {
                    /* CLOSED/UNKNOWN feedback keeps blocked result. */
                }
            }
        }
    }

    return isHandled;
}
