/**
 * @file    F01_InputMonitorAndValidator.c
 * @brief   F-01: Input Monitor and Validator Module - Implementation
 *
 * @details Implements the input collection and validation logic for the
 *          Electronic Child Lock system. Validates raw sensor signals
 *          (speed, crash, switch, rear door handle) and produces
 *          validated outputs, fault flags, and event flags.
 *
 *          Processing order per cycle (Run):
 *            1. NULL pointer guard
 *            2. Auto-initialize if needed
 *            3. Initialize output to safe defaults
 *            4. Ignition OFF guard (return safe defaults)
 *            5. Validate crash signal (D1)
 *            6. Validate speed signal (D2)
 *            7. Validate switch input (D3)
 *            8. Evaluate auto-lock condition (D6)
 *            9. Detect events (D7)
 *
 * @version 1.0.0
 * @date    2026-03-13
 * @author  AI Model (v1.0.0): Claude Opus 4.6
 * @copyright Synetics 20 CopyrightcSynetics_
 *
 * @req_id      REQ-SW-CL-001, REQ-SW-CL-002, REQ-SW-CL-003, REQ-SW-CL-004
 * @asil        ASIL B
 * @traceability DD-CL-F01, UC-1, UC-2, UC-3, UC-4
 *
 * @note    No dynamic memory allocation used.
 * @note    Complies with MISRA C:2012.
 */

#include "F01_InputMonitorAndValidator.h"

/* =========================================================================
 * Private Helper Functions (static)
 * ========================================================================= */

/**
 * @brief Initializes the validated output to safe defaults.
 *
 * @details All output fields are set to their safest possible values:
 *          no switch request, zero speed, no crash, no auto-lock,
 *          no events, no faults.
 *
 * @param[out] output  Pointer to the output structure. Must not be NULL.
 *
 * @traceability DD-CL-F01-DEFAULT
 */
static void initDefaultOutput(F01_ValidatedOutput_t * const output)
{
    output->validSwitchInput    = SWITCH_REQUEST_NONE;
    output->validSpeedKmh       = 0.0f;
    output->speedValid          = false;
    output->validCrashSignal    = false;
    output->autoLockConditionMet = false;
    output->eventFlags          = EVENT_FLAG_NONE;
    output->faultFlags          = FAULT_FLAG_NONE;
}

/**
 * @brief Validates the crash signal based on active + valid flags.
 *
 * @details A crash is considered valid only when BOTH the signal is active
 *          AND the source is marked trustworthy (e.g., airbag deployment
 *          confirmed). This prevents false crash detection from a single
 *          faulty flag.
 *
 * @param[in]  input  Raw input data.
 * @param[out] output Validated output (validCrashSignal field updated).
 *
 * @req_id REQ-SW-CL-003
 * @traceability DD-CL-F01-D1, UC-3
 */
static void validateCrashSignal(
    const F01_RawInput_t  * const input,
    F01_ValidatedOutput_t * const output)
{
    /* D1: CrashSignal is valid? Both conditions must be true */
    if (input->crashSignalActive && input->crashSignalValid)
    {
        output->validCrashSignal = true;
    }
    /* Otherwise, validCrashSignal remains false from initDefaultOutput */
}

/**
 * @brief Validates vehicle speed by range check and timeout check.
 *
 * @details Range: 0.0f <= speed <= SPEED_MAX_VALID_KMH (250 km/h).
 *          Timeout: (currentTimestamp - speedTimestamp) < SPEED_SIGNAL_TIMEOUT_MS.
 *          If either check fails, FAULT_FLAG_SPEED_FAULT is set and
 *          validSpeedKmh is forced to 0.0f for fail-safe behavior.
 *
 * @param[in]  input  Raw input data with speed and timestamps.
 * @param[out] output Validated output (speed fields + faultFlags updated).
 *
 * @req_id REQ-SW-CL-002
 * @traceability DD-CL-F01-D2, UC-2
 */
static void validateSpeed(
    const F01_RawInput_t  * const input,
    F01_ValidatedOutput_t * const output)
{
    bool rangeValid   = false;
    bool timeoutValid = false;

    /* Range check: speed must be within physically plausible bounds */
    if ((input->vehicleSpeedKmh >= 0.0f) &&
        (input->vehicleSpeedKmh <= SPEED_MAX_VALID_KMH))
    {
        rangeValid = true;
    }

    /* Timeout check: signal must not be stale beyond allowed window */
    {
        const uint32_t elapsed =
            input->currentTimestampMs - input->speedTimestampMs;

        if (elapsed < SPEED_SIGNAL_TIMEOUT_MS)
        {
            timeoutValid = true;
        }
    }

    /* Speed is valid only if both checks pass */
    if (rangeValid && timeoutValid)
    {
        output->validSpeedKmh = input->vehicleSpeedKmh;
        output->speedValid    = true;
    }
    else
    {
        /* D2 failure: speed signal invalid -> set fault flag */
        output->validSpeedKmh = 0.0f;
        output->speedValid    = false;
        output->faultFlags    = (FaultFlag_t)((uint8_t)output->faultFlags
                                | (uint8_t)FAULT_FLAG_SPEED_FAULT);
    }
}

/**
 * @brief Passes through the switch request as validated input.
 *
 * @details Debouncing is handled upstream per SDD specification.
 *          F-01 passes the request through without modification.
 *          Unlock inhibition while driving is F-02's responsibility.
 *
 * @param[in]  input  Raw input data with switch request.
 * @param[out] output Validated output (validSwitchInput field updated).
 *
 * @req_id REQ-SW-CL-001
 * @traceability DD-CL-F01-D3, UC-1
 */
static void validateSwitchInput(
    const F01_RawInput_t  * const input,
    F01_ValidatedOutput_t * const output)
{
    /* D3: SwitchInput event exists? Pass through if present */
    output->validSwitchInput = input->switchRequest;
}

/**
 * @brief Evaluates the auto-lock condition based on validated speed.
 *
 * @details Auto-lock condition is met when valid speed strictly exceeds
 *          the auto-lock threshold (> 3 km/h). At exactly 3.0 km/h,
 *          auto-lock is NOT triggered—complementary to F-02's unlock
 *          semantics where speed <= 3 allows unlock.
 *
 * @param[in,out] output Validated output (autoLockConditionMet updated).
 *                       Must have speedValid and validSpeedKmh set first.
 *
 * @req_id REQ-SW-CL-002
 * @traceability DD-CL-F01-D6, UC-2
 */
static void evaluateAutoLockCondition(F01_ValidatedOutput_t * const output)
{
    /* D6: Speed above auto-lock threshold? Only when speed is valid */
    if (output->speedValid &&
        (output->validSpeedKmh > (float)AUTO_LOCK_SPEED_THRESHOLD_KMH))
    {
        output->autoLockConditionMet = true;
    }
    /* Otherwise remains false from initDefaultOutput */
}

/**
 * @brief Detects discrete events from raw input signals.
 *
 * @details Currently detects rear door inner handle pull events.
 *          Extensible for future event types via bitmask OR.
 *
 * @param[in]  input  Raw input data with event signals.
 * @param[out] output Validated output (eventFlags field updated).
 *
 * @req_id REQ-SW-CL-004
 * @traceability DD-CL-F01-D7, UC-4
 */
static void detectEvents(
    const F01_RawInput_t  * const input,
    F01_ValidatedOutput_t * const output)
{
    /* D7: RearDoorInnerHandleEvent exists? */
    if (input->rearDoorInnerHandleActive)
    {
        output->eventFlags = (EventFlag_t)((uint8_t)output->eventFlags
                             | (uint8_t)EVENT_FLAG_REAR_DOOR_HANDLE);
    }
}

/* =========================================================================
 * Public API Implementation
 * ========================================================================= */

/**
 * @brief Initializes the F-01 context to a known safe state.
 *
 * @param[out] ctx  Caller-managed context. Must not be NULL.
 *
 * @req_id      REQ-SW-CL-001
 * @asil        ASIL B
 * @traceability DD-CL-F01-INIT
 */
void F01_InputMonitorAndValidator_Init(F01_Context_t * const ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    ctx->isInitialized = true;
}

/**
 * @brief Validates all raw inputs for one processing cycle.
 *
 * @details Steps:
 *   1. Validate pointers (NULL guard)
 *   2. Auto-init if context was not initialized
 *   3. Initialize output to safe defaults
 *   4. If ignition OFF, return safe defaults (no validation needed)
 *   5. Validate crash signal (D1)
 *   6. Validate speed signal (D2)
 *   7. Validate switch input (D3)
 *   8. Evaluate auto-lock condition (D6, depends on Step 6)
 *   9. Detect events (D7)
 *
 * @param[in,out] ctx    F01 context. Must not be NULL.
 * @param[in]     input  Raw input data. Must not be NULL.
 * @param[out]    output Validated output. Must not be NULL.
 *
 * @req_id      REQ-SW-CL-001, REQ-SW-CL-002
 * @asil        ASIL B
 * @traceability DD-CL-F01-RUN
 */
void F01_InputMonitorAndValidator_Run(
    F01_Context_t             * const ctx,
    const F01_RawInput_t      * const input,
    F01_ValidatedOutput_t     * const output)
{
    /* Step 1: Guard against NULL pointers */
    if ((ctx == NULL) || (input == NULL) || (output == NULL))
    {
        return;
    }

    /* Step 2: Auto-initialize if caller forgot Init */
    if (!ctx->isInitialized)
    {
        F01_InputMonitorAndValidator_Init(ctx);
    }

    /* Step 3: Initialize output to safe defaults */
    initDefaultOutput(output);

    /* Step 4: Ignition OFF guard - all outputs remain at safe defaults */
    if (input->ignitionState != IGN_STATE_ON)
    {
        return;
    }

    /* Step 5: Validate crash signal (D1) */
    validateCrashSignal(input, output);

    /* Step 6: Validate speed signal (D2) */
    validateSpeed(input, output);

    /* Step 7: Validate switch input (D3) */
    validateSwitchInput(input, output);

    /* Step 8: Evaluate auto-lock condition (D6, depends on Step 6) */
    evaluateAutoLockCondition(output);

    /* Step 9: Detect events (D7) */
    detectEvents(input, output);
}

/**
 * @brief Resets the F-01 context (e.g. on IGN cycle).
 *
 * @param[out] ctx  Context to reset. Must not be NULL.
 *
 * @req_id      REQ-SW-CL-001
 * @asil        ASIL B
 * @traceability DD-CL-F01-RESET
 */
void F01_InputMonitorAndValidator_Reset(F01_Context_t * const ctx)
{
    F01_InputMonitorAndValidator_Init(ctx);
}
