/**
 * @file    F08_RearRiskProtectionController.c
 * @brief   F-08: Rear Risk Protection Controller Module - Implementation
 *
 * @details Implements protection actions for UC-5.
 *          Forces Child Lock to ON if high risk is reported.
 *          Implements the SAFE_LOCKED fallback policy for sensor faults.
 *
 * @version 1.1.0
 * @date    2026-03-11
 * @author  AI Model: Gemini 3.5 Pro (review & fix)
 * @copyright Synetics 20 CopyrightⓒSynetics_
 *
 * @req_id  REQ-SW-ESA-002
 * @asil    ASIL B
 * @traceability DD-CL-F08, UC-5
 *
 * @note v1.1.0: Fixed warningSound=false->true on sensor fault path
 *              per SDD 5.2.3 SAFE_LOCKED policy (HMI alert mandatory).
 * @note Function length <= 80 lines. Cyclomatic Complexity <= 10.
 * @note No dynamic memory allocation.
 */

#include "F08_RearRiskProtectionController.h"

/* =========================================================================
 * Private Helper - Default Output initialization
 * ========================================================================= */

/**
 * @brief Initializes output to neutral/safe defaults.
 * @param[in]  currentCLState The current system child lock state.
 * @param[out] output         Pointer to output struct to initialize.
 */
static void initNeutralOutput(
    const ChildLockState_t       currentCLState,
    RearRiskProtectionOutput_t * const output)
{
    /* By default, we request no state change and no warnings */
    output->targetCLState = currentCLState;
    output->warningMsgId  = WARNING_MSG_NONE;
    output->warningSound  = false;
    output->clChanged     = false;
}

/* =========================================================================
 * Public API Implementation
 * ========================================================================= */

/**
 * @brief Executes the rear risk protection logic.
 *
 * @param[in]  input   Protection inputs (risk state, ign state, current CL state).
 * @param[out] output  Output actions (target CL state, warnings).
 */
void F08_RearRiskProtectionController_Run(
    const RearRiskProtectionInput_t  * const input,
    RearRiskProtectionOutput_t       * const output)
{
    /* Parameter sanity check (defensive programming) */
    if ((input == NULL) || (output == NULL))
    {
        return;
    }

    /* Start with a neutral baseline that requests no change */
    initNeutralOutput(input->currentCLState, output);

    /* Feature is only active when Ignition is ON */
    if (input->ignitionState != IGN_STATE_ON)
    {
        return;
    }

    /* Failure Mode Handling: If sensor result is invalid (faulty) */
    if (!input->riskValid)
    {
        /* Safe State Policy (SDD 5.2.3): SAFE_LOCKED -> Force CL ON.
         * HMI alert (sound + message) is MANDATORY even for faults
         * to ensure the driver is always informed of abnormal state. */
        output->targetCLState = CL_STATE_ON;
        output->warningMsgId  = WARNING_MSG_SENSOR_FAULT;
        output->warningSound  = true; /* Mandatory: driver must be alerted on fault */

        if (input->currentCLState == CL_STATE_OFF)
        {
            output->clChanged = true;
        }
        return;
    }

    /* Normal Operation: If risk is HIGH, take protective action */
    if (input->riskHigh)
    {
        output->targetCLState = CL_STATE_ON;
        output->warningSound  = true; /* Audible beep for imminent physical danger */

        /* Did we actively intervene, or was it already locked? */
        if (input->currentCLState == CL_STATE_OFF)
        {
            /* CL was OFF - we are auto-activating it to protect passenger */
            output->warningMsgId = WARNING_MSG_REAR_RISK_HIGH;
            output->clChanged    = true;
        }
        else
        {
            /* CL was already ON - maintain lock but still warn driver */
            output->warningMsgId = WARNING_MSG_CL_ALREADY_ON;
            output->clChanged    = false;
        }
    }
}
