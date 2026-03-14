/**
 * @file    F01_InputMonitorAndValidator.h
 * @brief   F-01: Input Monitor and Validator Module - Public Interface
 *
 * @details Collects raw switch input, vehicle speed, crash signal, rear door
 *          inner handle event, and ignition state. Validates each signal and
 *          produces validated outputs, fault flags, and event flags for
 *          downstream decision logic (F-02 ChildLockStateDecision).
 *
 *          Validation includes:
 *            - Speed: range check (0 ~ 250 km/h) and timeout (500ms)
 *            - Crash signal: active + valid flag conjunction
 *            - Switch input: pass-through (debouncing handled upstream)
 *            - Rear door inner handle: event detection
 *            - Auto-lock condition: speed > threshold evaluation
 *
 *          Failure Mode Responses:
 *            - Speed out of range or timed out -> FAULT_FLAG_SPEED_FAULT
 *            - Invalid crash signal -> validCrashSignal = false (no fault flag)
 *
 * @version 1.0.0
 * @date    2026-03-13
 * @author  AI Model (v1.0.0): Claude Opus 4.6
 * @copyright Synetics 20 CopyrightcSynetics_
 *
 * @req_id      REQ-SW-CL-001, REQ-SW-CL-002
 * @asil        ASIL B
 * @traceability DD-CL-F01, UC-1, UC-2, UC-3, UC-4
 *
 * @note    No dynamic memory allocation used.
 * @note    Complies with MISRA C:2012.
 */

#ifndef F01_INPUT_MONITOR_AND_VALIDATOR_H
#define F01_INPUT_MONITOR_AND_VALIDATOR_H

#include "childlock_types.h"

/* =========================================================================
 * Internal Context Structure (caller-managed, stack-allocated)
 * ========================================================================= */

/**
 * @brief Internal state context for F-01 InputMonitorAndValidator.
 *
 * @details The caller must allocate this on the stack or as a static variable
 *          and pass it to F01_InputMonitorAndValidator_Init() before first use.
 *          Do NOT modify fields directly; use the provided API.
 *
 * @note    No heap allocation. Lifetime managed by the caller.
 */
typedef struct
{
    bool isInitialized; /**< Initialization guard flag. */
} F01_Context_t;

/* =========================================================================
 * Public API
 * ========================================================================= */

/**
 * @brief Initializes the F-01 context to a safe, known state.
 *
 * @details Must be called once before the first call to
 *          F01_InputMonitorAndValidator_Run().
 *
 * @param[out] ctx  Pointer to the caller-allocated F01_Context_t structure.
 *                  Must not be NULL.
 *
 * @return void
 *
 * @req_id      REQ-SW-CL-001
 * @asil        ASIL B
 * @traceability DD-CL-F01-INIT
 */
void F01_InputMonitorAndValidator_Init(F01_Context_t * const ctx);

/**
 * @brief Validates all raw inputs for one processing cycle.
 *
 * @details Validates speed (range + timeout), crash signal (active + valid),
 *          passes through switch input, detects events, evaluates auto-lock
 *          condition, and generates fault/event flags.
 *
 * @param[in,out] ctx    Pointer to the persistent F01 context. Must not be NULL.
 * @param[in]     input  Raw input data for this cycle. Must not be NULL.
 * @param[out]    output Validated output data. Must not be NULL.
 *
 * @return void
 *
 * @req_id      REQ-SW-CL-001, REQ-SW-CL-002
 * @asil        ASIL B
 * @traceability DD-CL-F01-RUN
 *
 * @warning If ctx is uninitialized (ctx->isInitialized == false), the function
 *          will auto-initialize and produce valid output.
 */
void F01_InputMonitorAndValidator_Run(
    F01_Context_t             * const ctx,
    const F01_RawInput_t      * const input,
    F01_ValidatedOutput_t     * const output
);

/**
 * @brief Resets the F-01 context to initial state (e.g. on IGN OFF/ON cycle).
 *
 * @param[out] ctx  Pointer to the F01_Context_t to reset. Must not be NULL.
 *
 * @return void
 *
 * @req_id      REQ-SW-CL-001
 * @asil        ASIL B
 * @traceability DD-CL-F01-RESET
 */
void F01_InputMonitorAndValidator_Reset(F01_Context_t * const ctx);

#endif /* F01_INPUT_MONITOR_AND_VALIDATOR_H */
