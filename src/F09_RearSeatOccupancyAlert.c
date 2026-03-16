/**
 * @file    F09_RearSeatOccupancyAlert.c
 * @brief   F-09: Rear Seat Occupancy Alert Module - Implementation
 *
 * @version 1.1.0
 * @date    2026-03-14
 * @author  AI Model: Gemini
 * @copyright Synetics 20 CopyrightⓒSynetics_
 *
 * @req_id      REQ-SW-ESA-003
 * @asil        ASIL B
 * @traceability DD-CL-F09, UC-6
 */

#include "F09_RearSeatOccupancyAlert.h"
#include <stddef.h>

/* =========================================================================
 * Private Helper - Output Initialization
 * ========================================================================= */

/**
 * @brief Initializes output to neutral/safe defaults.
 */
static void initNeutralOutput(F09_OccupancyAlertOutput_t * const output)
{
    output->alertType      = F09_ALERT_TYPE_NONE;
    output->alertMessageId = WARNING_MSG_NONE;
    output->eventLog       = false;
}

/* =========================================================================
 * Public API Implementation
 * ========================================================================= */

void F09_RearSeatOccupancyAlert_Run(
    const F09_OccupancyAlertInput_t  * const input,
    F09_OccupancyAlertOutput_t       * const output)
{
    /* Parameter sanity check (defensive programming) */
    if ((input == NULL) || (output == NULL))
    {
        return;
    }

    /* Start with a neutral baseline */
    initNeutralOutput(output);

    /* Feature requires an active departure event and healthy HMI */
    if ((!input->departureEvent) || (!input->hmiHealth))
    {
        return;
    }

    if (input->rearSeatOccupancy)
    {
        output->eventLog = true; 

        if (input->currentCLState == CL_STATE_OFF)
        {
            output->alertType      = F09_ALERT_TYPE_RECOMMENDATION;
            output->alertMessageId = WARNING_MSG_NONE; /* Using existing enum only */
        }
        else
        {
            output->alertType      = F09_ALERT_TYPE_STATUS;
            output->alertMessageId = WARNING_MSG_CHILD_LOCK_ON;
        }
    }
    else
    {
        output->eventLog = true; 
    }
}