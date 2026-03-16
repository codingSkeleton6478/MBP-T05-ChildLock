#ifndef F02_CHILD_LOCK_STATE_DECISION_H
#define F02_CHILD_LOCK_STATE_DECISION_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Child lock on/off state used as the commanded lock state.
 * @note SAFE_LOCKED / EMERGENCY_RELEASED are intentionally not modeled as
 *       separate states because the specification is not explicit.
 * @req_id UC-1, UC-2, UC-3
 * @traceability FG-01, F-02, SwDD 5.1.2, SwDD 5.1.3
 * @asil specification not explicit in repository documents
 */
typedef enum ChildLockState
{
    CL_OFF = 0U,
    CL_ON
} ChildLockState;

/**
 * @brief Draft reason codes for state decision results.
 * @note TODO: official reason taxonomy is not defined in the current documents.
 */
typedef enum DecisionReason
{
    DECISION_NO_STATE_CHANGE = 0U,
    DECISION_CRASH_DETECTED,
    DECISION_FAULT_DETECTED,
    DECISION_DRIVER_LOCK_REQUEST,
    DECISION_DRIVER_UNLOCK_REQUEST,
    DECISION_UNLOCK_INHIBITED_WHILE_MOVING,
    DECISION_AUTO_LOCK_CONDITION_MET
} DecisionReason;

/**
 * @brief Draft safety action codes accompanying the target state.
 * @note TODO: official action taxonomy is not defined in the current documents.
 */
typedef enum SafetyAction
{
    SAFETY_NONE = 0U,
    SAFETY_ENTER_SAFE_LOCKED,
    SAFETY_ENTER_EMERGENCY_RELEASED,
    SAFETY_INHIBIT_UNLOCK
} SafetyAction;

/**
 * @brief Validated driver request handed over from input validation logic.
 * @note TODO: long-press / dual-confirm semantics are specification not explicit
 *       for F-02 level code and should be handled before this decision function.
 */
typedef enum DriverRequest
{
    DRIVER_NONE = 0U,
    DRIVER_REQ_ON,
    DRIVER_REQ_OFF
} DriverRequest;

/**
 * @brief Fault flags relevant to the F-02 state decision.
 * @note TODO: exact fault bit allocation is specification not explicit.
 */
typedef struct FaultFlags
{
    bool speedSignalFault;
    bool sensorFault;
    bool stateInconsistency;
} FaultFlags;

/**
 * @brief Inputs consumed by the child lock state decision.
 * @note AUTO_LOCK_SPEED_THRESHOLD value is specification not explicit here.
 *       The caller provides the already-evaluated auto-lock condition.
 */
typedef struct DecisionInput
{
    DriverRequest validSwitchInput;
    float vehicleSpeedKmh;
    bool validCrashSignal;
    ChildLockState currentClState;
    FaultFlags faultFlags;
    bool autoLockConditionMet;
} DecisionInput;

/**
 * @brief Output of the F-02 child lock state decision.
 * @note Emergency release and safe-state semantics are represented by
 *       reason/action fields rather than extending ChildLockState.
 */
typedef struct DecisionResult
{
    ChildLockState targetClState;
    DecisionReason decisionReason;
    SafetyAction safetyAction;
    bool stateChanged;
} DecisionResult;

/**
 * @brief Determine the target child lock state from validated inputs.
 * @param input Pointer to validated decision inputs from F-01 and current state context.
 * @return DecisionResult containing target state, reason, and safety action.
 * @req_id UC-1, UC-2, UC-3
 * @traceability FG-01, F-02, SwDD 5.1.2, SwDD 5.1.3, Failure_Mode UC-1..3
 * @asil specification not explicit in repository documents
 * @note Priority order: Crash > Fault > Driver Input > Auto Lock.
 * @note Unlock requests while driving above 3 km/h are inhibited.
 */
DecisionResult DecideChildLockState(const DecisionInput* input);

#ifdef __cplusplus
}
#endif

#endif  /* F02_CHILD_LOCK_STATE_DECISION_H */
