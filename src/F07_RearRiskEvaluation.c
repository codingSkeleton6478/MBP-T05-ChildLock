/**
 * @file    F07_RearRiskEvaluation.c
 * @brief   F-07: Rear Risk Evaluation Module - Implementation
 *
 * @details Implements the rear approach risk evaluation for UC-5.
 *          Evaluates danger based on three simultaneous conditions:
 *            1. Distance <= REAR_RISK_DIST_THRESHOLD_M
 *            2. Relative speed >= REAR_RISK_RELSPEED_THRESHOLD_MPS (approaching)
 *            3. Sensor confidence >= REAR_RISK_SENSOR_CONFIDENCE_MIN
 *
 *          Hysteresis logic:
 *            - Risk HIGH confirmed after REAR_RISK_CONFIRM_DURATION_MS
 *            - Risk HIGH cleared after REAR_RISK_CLEAR_DURATION_MS (safe period)
 *
 *          Failure Mode Response (Failure_Mode.md - UC-5 / B / Dooring Risk):
 *            - Sensor fault  → riskValid=false, faultFlag=FAULT_FLAG_REAR_SENSOR
 *            - Invalid input → defaults to safe (no false activation)
 *
 * @version 1.0.0
 * @date    2026-03-11
 * @author  AI Model: Gemini 3.5 Pro
 * @copyright Synetics 20 CopyrightⓒSynetics_
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07, UC-5
 *
 * @note    Function length <= 80 lines. Cyclomatic Complexity <= 10.
 * @note    No dynamic memory allocation (MISRA C:2012 Rule 21.3).
 * @note    All inputs validated before use.
 */

#include "F07_RearRiskEvaluation.h"

/* =========================================================================
 * Private Helper - Input Validation
 * ========================================================================= */

/**
 * @brief Validates that the raw sensor inputs are within physically plausible bounds.
 *
 * @details Returns false if:
 *          - sensorHealth is false (hardware fault)
 *          - sensorConfidence < REAR_RISK_SENSOR_CONFIDENCE_MIN
 *          - distanceM < 0.0 or > REAR_RISK_DIST_MAX_VALID_M
 *          - relSpeedMps < 0.0 or > REAR_RISK_RELSPEED_MAX_VALID_MPS
 *
 * @param[in] input  Pointer to the sensor input structure.
 * @return true if all fields are within valid ranges, false otherwise.
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-VAL
 */
static bool isInputValid(const RearRiskInput_t * const input)
{
    /* Check sensor hardware health flag */
    if (!input->sensorHealth)
    {
        return false;
    }

    /* Check sensor confidence meets minimum threshold */
    if (input->sensorConfidence < REAR_RISK_SENSOR_CONFIDENCE_MIN)
    {
        return false;
    }

    /* Validate distance is within physical range */
    if ((input->distanceM < 0.0f) || (input->distanceM > REAR_RISK_DIST_MAX_VALID_M))
    {
        return false;
    }

    /* Validate relative speed is within physical range */
    if ((input->relSpeedMps < 0.0f) || (input->relSpeedMps > REAR_RISK_RELSPEED_MAX_VALID_MPS))
    {
        return false;
    }

    return true;
}

/* =========================================================================
 * Private Helper - Raw Threat Condition Check
 * ========================================================================= */

/**
 * @brief Checks whether raw threshold conditions for danger are currently met.
 *
 * @details A raw threat exists when BOTH of the following are true:
 *          - Distance is below the danger threshold
 *          - Approach speed equals or exceeds the danger threshold
 *
 * @param[in] input  Pointer to validated sensor input.
 * @return true if raw threat conditions are satisfied, false otherwise.
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-THRESH
 */
static bool isRawThreatActive(const RearRiskInput_t * const input)
{
    /* Condition 1: Object is within the risk distance threshold */
    const bool isClose = (input->distanceM <= REAR_RISK_DIST_THRESHOLD_M);

    /* Condition 2: Object is approaching fast enough to be dangerous */
    const bool isFast  = (input->relSpeedMps >= REAR_RISK_RELSPEED_THRESHOLD_MPS);

    return (isClose && isFast);
}

/* =========================================================================
 * Private Helper - Risk Level Classification
 * ========================================================================= */

/**
 * @brief Classifies the risk level based on distance and approach speed.
 *
 * @details Four levels: NONE, LOW, MEDIUM, HIGH.
 *          HIGH is only returned when both thresholds are breached.
 *
 * @param[in] input  Pointer to validated sensor input.
 * @return RiskLevel_t classification for the current measurement.
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-CLASSIFY
 */
static RiskLevel_t classifyRiskLevel(const RearRiskInput_t * const input)
{
    /* Danger zone: both conditions met */
    if ((input->distanceM <= REAR_RISK_DIST_THRESHOLD_M) &&
        (input->relSpeedMps >= REAR_RISK_RELSPEED_THRESHOLD_MPS))
    {
        return RISK_LEVEL_HIGH;
    }

    /* Medium: close but not approaching fast, or fast but not yet close */
    if ((input->distanceM <= REAR_RISK_DIST_THRESHOLD_M) ||
        (input->relSpeedMps >= REAR_RISK_RELSPEED_THRESHOLD_MPS))
    {
        return RISK_LEVEL_MEDIUM;
    }

    /* Low: object present but not threatening (distance up to 2x threshold) */
    if (input->distanceM <= (REAR_RISK_DIST_THRESHOLD_M * 2.0f))
    {
        return RISK_LEVEL_LOW;
    }

    /* None: object is far or not detected */
    return RISK_LEVEL_NONE;
}

/* =========================================================================
 * Private Helper - Hysteresis State Machine
 * ========================================================================= */

/**
 * @brief Updates the hysteresis state machine for HIGH risk confirmation/clearing.
 *
 * @details Transitions:
 *          - Threat active + elapsed >= CONFIRM → riskHigh = true
 *          - Threat inactive + elapsed >= CLEAR  → riskHigh = false
 *          This prevents rapid toggling on borderline conditions.
 *
 * @param[in,out] ctx           F07 context (state updated in-place).
 * @param[in]     rawThreat     Current raw threat condition result.
 * @param[in]     nowMs         Current system timestamp (ms).
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-HYSTERESIS
 */
static void updateHysteresis(
    F07_Context_t * const ctx,
    const bool            rawThreat,
    const uint32_t        nowMs)
{
    if (rawThreat)
    {
        /* Reset the clear-timer whenever threat is present */
        ctx->riskClearTimestampMs = nowMs;

        if (!ctx->isThreatOngoing)
        {
            /* First cycle threat detected: start onset timer */
            ctx->isThreatOngoing       = true;
            ctx->riskOnsetTimestampMs  = nowMs;
        }

        /* Confirm HIGH only after sustained threat */
        const uint32_t onsetElapsed = nowMs - ctx->riskOnsetTimestampMs;
        if (onsetElapsed >= REAR_RISK_CONFIRM_DURATION_MS)
        {
            ctx->isRiskHighActive = true;
        }
    }
    else
    {
        /* Reset the onset-timer whenever threat is absent */
        ctx->riskOnsetTimestampMs = nowMs;
        ctx->isThreatOngoing      = false;

        /* Clear HIGH only after sustained safe condition */
        const uint32_t clearElapsed = nowMs - ctx->riskClearTimestampMs;
        if (clearElapsed >= REAR_RISK_CLEAR_DURATION_MS)
        {
            ctx->isRiskHighActive = false;
        }
    }
}

/* =========================================================================
 * Public API Implementation
 * ========================================================================= */

/**
 * @brief Initializes the F-07 context to a known safe state.
 * @param[out] ctx  Caller-managed context. Must not be NULL.
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-INIT
 */
void F07_RearRiskEvaluation_Init(F07_Context_t * const ctx)
{
    /* NULL guard: do nothing if caller passes invalid pointer */
    if (ctx == NULL)
    {
        return;
    }

    ctx->riskOnsetTimestampMs = 0U;
    ctx->riskClearTimestampMs = 0U;
    ctx->isRiskHighActive     = false;
    ctx->isThreatOngoing      = false;
    ctx->isInitialized        = true;
}

/**
 * @brief Runs the rear risk evaluation for one processing cycle.
 *
 * @details Steps:
 *          1. Validate pointers
 *          2. Auto-init if not yet initialized
 *          3. Validate sensor input range/health
 *          4. If invalid → set fault output and return
 *          5. Check raw threshold conditions
 *          6. Update hysteresis state
 *          7. Classify risk level
 *          8. Write result to output
 *
 * @param[in]  ctx     F07 context (persistent state). Must not be NULL.
 * @param[in]  input   Sensor data for this cycle. Must not be NULL.
 * @param[out] output  Evaluation result. Must not be NULL.
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-RUN
 */
void F07_RearRiskEvaluation_Run(
    F07_Context_t         * const ctx,
    const RearRiskInput_t * const input,
    RearRiskOutput_t      * const output)
{
    /* Step 1: Guard against NULL pointers (defensive programming) */
    if ((ctx == NULL) || (input == NULL) || (output == NULL))
    {
        return;
    }

    /* Step 2: Auto-initialize if caller forgot to call Init */
    if (!ctx->isInitialized)
    {
        F07_RearRiskEvaluation_Init(ctx);
    }

    /* Step 3: Validate sensor data range and health */
    if (!isInputValid(input))
    {
        /* Step 4: Sensor fault path → mark output as invalid, set fault */
        output->riskHigh  = false;
        output->riskLevel = RISK_LEVEL_NONE;
        output->riskValid = false;
        output->faultFlag = FAULT_FLAG_REAR_SENSOR;
        /* Safe State per SDD 5.2.3: SAFE_LOCKED → CL ON (handled by F-08) */
        return;
    }

    /* Step 5: Check raw threshold conditions */
    const bool rawThreat = isRawThreatActive(input);

    /* Step 6: Update hysteresis state machine */
    updateHysteresis(ctx, rawThreat, input->timestampMs);

    /* Step 7: Classify risk level (independent of hysteresis) */
    const RiskLevel_t level = classifyRiskLevel(input);

    /* Step 8: Write result — riskHigh uses hysteresis-confirmed state */
    output->riskHigh  = ctx->isRiskHighActive;
    output->riskLevel = level;
    output->riskValid = true;
    output->faultFlag = FAULT_FLAG_NONE;
}

/**
 * @brief Resets the F-07 context (e.g. on IGN cycle).
 * @param[out] ctx  Context to reset. Must not be NULL.
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-RESET
 */
void F07_RearRiskEvaluation_Reset(F07_Context_t * const ctx)
{
    /* Delegate to Init for a clean reset */
    F07_RearRiskEvaluation_Init(ctx);
}
