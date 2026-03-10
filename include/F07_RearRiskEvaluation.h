/**
 * @file    F07_RearRiskEvaluation.h
 * @brief   F-07: Rear Risk Evaluation Module - Public Interface
 *
 * @details Evaluates the risk level from a rear-approaching object based on:
 *            - Distance to the object (m)
 *            - Relative approach speed (m/s)
 *            - Duration the threat condition has persisted (ms)
 *            - Sensor health and confidence level
 *
 *          Implements hysteresis to prevent false triggering:
 *            - Risk transitions to HIGH only after REAR_RISK_CONFIRM_DURATION_MS
 *            - Risk clears from HIGH only after REAR_RISK_CLEAR_DURATION_MS
 *
 *          If the sensor is unhealthy, the output is marked invalid and
 *          FAULT_FLAG_REAR_SENSOR is set (Safe State = SAFE_LOCKED per SDD 5.2.3).
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
 * @note    No dynamic memory allocation used.
 * @note    Complies with MISRA C:2012.
 */

#ifndef F07_REAR_RISK_EVALUATION_H
#define F07_REAR_RISK_EVALUATION_H

#include "childlock_types.h"

/* =========================================================================
 * Internal Context Structure (caller-managed, stack-allocated)
 * ========================================================================= */

/**
 * @brief Internal state context for F-07 hysteresis tracking.
 *
 * @details The caller must allocate this on the stack or as a static variable
 *          and pass it to F07_RearRiskEvaluation_Init() before first use.
 *          Do NOT modify fields directly; use the provided API.
 *
 * @note    No heap allocation. Lifetime managed by the caller.
 */
typedef struct
{
    uint32_t riskOnsetTimestampMs;  /**< Timestamp when risk condition first met.       */
    uint32_t riskClearTimestampMs;  /**< Timestamp when safe condition first detected.  */
    bool     isRiskHighActive;      /**< Current confirmed HIGH risk state.             */
    bool     isThreatOngoing;       /**< True while raw threat conditions are met.      */
    bool     isInitialized;         /**< Initialization guard flag.                     */
} F07_Context_t;

/* =========================================================================
 * Public API
 * ========================================================================= */

/**
 * @brief Initializes the F-07 context to a safe, known state.
 *
 * @details Must be called once before the first call to F07_RearRiskEvaluation_Run().
 *          Resets all hysteresis counters and flags.
 *
 * @param[out] ctx  Pointer to the caller-allocated F07_Context_t structure.
 *                  Must not be NULL.
 *
 * @return void
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-INIT
 */
void F07_RearRiskEvaluation_Init(F07_Context_t * const ctx);

/**
 * @brief Evaluates rear approach risk from sensor input each cycle.
 *
 * @details Determines the risk level based on distance, relative speed,
 *          sensor health/confidence, and temporal hysteresis. The result
 *          is written to 'output'. If the sensor is faulty, riskValid is
 *          set to false and faultFlag is set to FAULT_FLAG_REAR_SENSOR.
 *
 * @param[in]  ctx     Pointer to the persistent F07 context. Must not be NULL.
 * @param[in]  input   Sensor data for this evaluation cycle. Must not be NULL.
 * @param[out] output  Evaluated risk result. Must not be NULL.
 *
 * @return void
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-RUN
 *
 * @warning If ctx is uninitialized (ctx->isInitialized == false), the function
 *          will auto-initialize and set output to a safe default (riskHigh=false).
 */
void F07_RearRiskEvaluation_Run(
    F07_Context_t     * const ctx,
    const RearRiskInput_t   * const input,
    RearRiskOutput_t        * const output
);

/**
 * @brief Resets the F-07 context to initial state (e.g. on IGN OFF/ON cycle).
 *
 * @param[out] ctx  Pointer to the F07_Context_t to reset. Must not be NULL.
 *
 * @return void
 *
 * @req_id      REQ-SW-ESA-001
 * @asil        ASIL B
 * @traceability DD-CL-F07-RESET
 */
void F07_RearRiskEvaluation_Reset(F07_Context_t * const ctx);

#endif /* F07_REAR_RISK_EVALUATION_H */
