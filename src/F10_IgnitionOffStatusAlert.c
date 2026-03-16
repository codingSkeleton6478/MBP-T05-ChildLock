/**
 * @file F10_IgnitionOffStatusAlert.c
 * @brief Minimal C11 implementation for F-10 IgnitionOffStatusAlert.
 *
 * @details This implementation evaluates the child lock state when IGN OFF
 *          occurs and emits a single UC-7 summary alert when appropriate.
 *          The alert-once policy is held in the context so repeated handling
 *          does not duplicate the same status output.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */

#include "F10_IgnitionOffStatusAlert.h"

#include <stddef.h>

/**
 * @brief Resets output fields to safe defaults.
 *
 * @param[out] output Pointer to the output structure.
 */
static void IgnitionOffStatusAlert_ClearOutput(IgnitionOffStatusAlert_Output_t *output)
{
    if (output != NULL)
    {
        output->statusSummaryMessage = IOA_STATUS_MESSAGE_NONE;
        output->alertPriority = IOA_ALERT_PRIORITY_NONE;
        output->eventLog = false;
        output->alertIssued = false;
    }
}

/**
 * @brief Validates a child lock state value returned by the query callback.
 *
 * @param[in] state Candidate child lock state.
 * @return true when the state matches the supported enum values.
 */
static bool IgnitionOffStatusAlert_IsValidState(ChildLockState_t state)
{
    return ((state == CL_STATE_OFF) || (state == CL_STATE_ON));
}

/**
 * @brief Validates mandatory dependencies before initialization succeeds.
 *
 * @param[in] config Candidate configuration.
 * @return true when required callbacks are present.
 */
static bool IgnitionOffStatusAlert_HasValidConfig(
    const IgnitionOffStatusAlert_Config_t *config)
{
    return ((config != NULL) && (config->queryChildLockState != NULL));
}

/**
 * @brief Verifies shared public-function preconditions.
 *
 * @param[in] alert Runtime context.
 * @param[in] input Event input.
 * @param[in] output Handler output.
 * @return true when the handler can safely proceed.
 */
static bool IgnitionOffStatusAlert_IsReady(const IgnitionOffStatusAlert_t *alert,
                                           const IgnitionOffStatusAlert_Input_t *input,
                                           const IgnitionOffStatusAlert_Output_t *output)
{
    return ((alert != NULL) && (input != NULL) && (output != NULL)
        && (alert->isInitialized == true));
}

/**
 * @brief Fills the output with a status alert and marks it loggable.
 *
 * @param[out] output Output structure to update.
 * @param[in] message Message selected for the alert.
 */
static void IgnitionOffStatusAlert_IssueAlert(IgnitionOffStatusAlert_Output_t *output,
                                              IgnitionOffStatusAlert_Message_t message)
{
    if (output != NULL)
    {
        output->statusSummaryMessage = message;
        output->alertPriority = IOA_ALERT_PRIORITY_STATUS;
        output->eventLog = true;
        output->alertIssued = true;
    }
}

/**
 * @brief Issues an alert and latches the alert-once policy state.
 *
 * @param[in,out] alert Runtime context to update.
 * @param[out] output Output structure to update.
 * @param[in] message Message selected for the alert.
 */
static void IgnitionOffStatusAlert_IssueLatchedAlert(IgnitionOffStatusAlert_t *alert,
                                                     IgnitionOffStatusAlert_Output_t *output,
                                                     IgnitionOffStatusAlert_Message_t message)
{
    if (alert != NULL)
    {
        IgnitionOffStatusAlert_IssueAlert(output, message);
        alert->alertAlreadyIssued = true;
    }
}

/**
 * @brief Initializes the F-10 context and resets alert-once state.
 *
 * @param[out] alert Pointer to the runtime context.
 * @param[in] config Pointer to the dependency configuration.
 * @return true if initialization succeeded, otherwise false.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
// cppcheck-suppress unusedFunction
bool IgnitionOffStatusAlert_Init(IgnitionOffStatusAlert_t *alert,
                                 const IgnitionOffStatusAlert_Config_t *config)
{
    bool isInitialized = false;

    if (alert != NULL)
    {
        alert->isInitialized = false;
        alert->alertAlreadyIssued = false;
    }

    if ((alert != NULL) && (IgnitionOffStatusAlert_HasValidConfig(config) == true))
    {
        alert->config = *config;
        alert->isInitialized = true;
        alert->alertAlreadyIssued = false;
        isInitialized = true;
    }

    return isInitialized;
}

/**
 * @brief Handles IGN OFF status alert generation for UC-7.
 *
 * @param[in] alert Pointer to the initialized runtime context.
 * @param[in] input Pointer to the current IGN OFF handling inputs.
 * @param[out] output Pointer receiving the selected alert output.
 * @return true if the request was processed, otherwise false.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
// cppcheck-suppress unusedFunction
bool IgnitionOffStatusAlert_HandleEvent(IgnitionOffStatusAlert_t *alert,
                                        const IgnitionOffStatusAlert_Input_t *input,
                                        IgnitionOffStatusAlert_Output_t *output)
{
    bool isHandled = false;
    ChildLockState_t currentState = CL_STATE_OFF;
    bool querySucceeded = false;

    IgnitionOffStatusAlert_ClearOutput(output);

    if (IgnitionOffStatusAlert_IsReady(alert, input, output) == true)
    {
        isHandled = true;

        if ((input->ignitionOffEvent == true) && (alert->alertAlreadyIssued == false))
        {
            querySucceeded = alert->config.queryChildLockState(&currentState);

            if ((querySucceeded == false)
                || (IgnitionOffStatusAlert_IsValidState(currentState) == false))
            {
                IgnitionOffStatusAlert_IssueLatchedAlert(
                    alert,
                    output,
                    IOA_STATUS_MESSAGE_QUERY_FALLBACK);
            }
            else if (currentState == CL_STATE_ON)
            {
                IgnitionOffStatusAlert_IssueLatchedAlert(
                    alert,
                    output,
                    IOA_STATUS_MESSAGE_CL_ON_SUMMARY);
            }
            else
            {
                /* Child lock OFF intentionally emits no status summary alert. */
            }
        }
    }

    return isHandled;
}
