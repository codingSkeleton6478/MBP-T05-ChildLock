/**
 * @file    F08_RearRiskProtectionController.c
 * @brief   F-08: Rear Risk Protection Controller Module - Implementation
 *
 * @details Implements protection actions for UC-5.
 *          Forces Child Lock to ON if high risk is reported.
 *          Implements the SAFE_LOCKED fallback policy for sensor faults.
 *
 * @version 1.2.0
 * @date    2026-03-11 01:42 (KST)
 * @author  AI Model (v1.0.0): Gemini 3.5 Pro — Initial implementation
 * @author  AI Model (v1.1.0~1.2.0): Claude Sonnet 4.6 (Thinking) — Review & fix
 * @copyright Synetics 20 CopyrightⓒSynetics_
 *
 * @req_id  REQ-SW-ESA-002
 * @asil    ASIL B
 * @traceability DD-CL-F08, UC-5
 *
 * @note v1.2.0: Added eventLog=true in sensor-fault and risk-HIGH branches
 *              per SDD Table 3.2 F-08 Output spec, FG-02 flowchart
 *              ('로그/DTC 저장'), and SD-5 sequence diagram
 *              ('자동 보호 이벤트 로그 기록').
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
    /* By default, we request no state change, no warnings, no log */
    output->targetCLState = currentCLState;
    output->warningMsgId  = WARNING_MSG_NONE;
    output->warningSound  = false;
    output->clChanged     = false;
    output->eventLog      = false; /* No loggable event until a branch sets it */
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
        output->warningSound  = true;  /* Mandatory: driver must be alerted on fault */
        output->eventLog      = true;  /* FG-02: "로그/DTC 저장" on Sensor Fault path */

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
        output->warningSound  = true;  /* Audible beep for imminent physical danger */
        output->eventLog      = true;  /* SD-5: "자동 보호 이벤트 로그 기록" */

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
