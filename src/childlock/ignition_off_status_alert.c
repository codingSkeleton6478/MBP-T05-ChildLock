/**
 * @file ignition_off_status_alert.c
 * @brief Stub implementation for F-10 IgnitionOffStatusAlert groundwork.
 *
 * @details This file provides defensive scaffolding for the public API while
 *          the production alert behavior is still being driven by tests.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */

#include "childlock/ignition_off_status_alert.h"

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

bool IgnitionOffStatusAlert_Init(IgnitionOffStatusAlert_t *alert,
                                 const IgnitionOffStatusAlert_Config_t *config)
{
    bool isInitialized = false;

    if ((alert != NULL) && (config != NULL))
    {
        alert->config = *config;
        alert->isInitialized = true;
        alert->alertAlreadyIssued = false;
        isInitialized = true;
    }

    return isInitialized;
}

bool IgnitionOffStatusAlert_HandleEvent(IgnitionOffStatusAlert_t *alert,
                                        const IgnitionOffStatusAlert_Input_t *input,
                                        IgnitionOffStatusAlert_Output_t *output)
{
    bool isHandled = false;

    IgnitionOffStatusAlert_ClearOutput(output);

    if ((alert != NULL) && (input != NULL) && (output != NULL)
        && (alert->isInitialized == true))
    {
        /* TODO(SDD-F-10): Apply IGN OFF summary alert rules and alert-once policy. */
        if (input->ignitionOffEvent == true)
        {
            isHandled = true;
        }
    }

    return isHandled;
}
