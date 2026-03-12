#ifndef CHILD_LOCK_STATE_DECISION_HPP
#define CHILD_LOCK_STATE_DECISION_HPP

#include <cstdint>

namespace child_lock
{

/**
 * @brief Child lock on/off state used as the commanded lock state.
 * @note SAFE_LOCKED / EMERGENCY_RELEASED are intentionally not modeled as
 *       separate states because the specification is not explicit.
 * @req_id UC-1, UC-2, UC-3
 * @traceability FG-01, F-02, SwDD 5.1.2, SwDD 5.1.3
 * @asil specification not explicit in repository documents
 */
enum class ChildLockState : std::uint8_t
{
    Off = 0U,
    On
};

/**
 * @brief Draft reason codes for state decision results.
 * @note TODO: official reason taxonomy is not defined in the current documents.
 */
enum class DecisionReason : std::uint8_t
{
    NoStateChange = 0U,
    CrashDetected,
    FaultDetected,
    DriverLockRequest,
    DriverUnlockRequest,
    UnlockInhibitedWhileMoving,
    AutoLockConditionMet
};

/**
 * @brief Draft safety action codes accompanying the target state.
 * @note TODO: official action taxonomy is not defined in the current documents.
 */
enum class SafetyAction : std::uint8_t
{
    None = 0U,
    EnterSafeLocked,
    EnterEmergencyReleased,
    InhibitUnlock
};

/**
 * @brief Validated driver request handed over from input validation logic.
 * @note TODO: long-press / dual-confirm semantics are specification not explicit
 *       for F-02 level code and should be handled before this decision function.
 */
enum class DriverRequest : std::uint8_t
{
    None = 0U,
    RequestOn,
    RequestOff
};

/**
 * @brief Fault flags relevant to the F-02 state decision.
 * @note TODO: exact fault bit allocation is specification not explicit.
 */
struct FaultFlags
{
    bool speedSignalFault;
    bool sensorFault;
    bool stateInconsistency;
};

/**
 * @brief Inputs consumed by the child lock state decision.
 * @note AUTO_LOCK_SPEED_THRESHOLD value is specification not explicit here.
 *       The caller provides the already-evaluated auto-lock condition.
 */
struct DecisionInput
{
    DriverRequest validSwitchInput;
    float vehicleSpeedKmh;
    bool validCrashSignal;
    ChildLockState currentClState;
    FaultFlags faultFlags;
    bool autoLockConditionMet;
};

/**
 * @brief Output of the F-02 child lock state decision.
 * @note Emergency release and safe-state semantics are represented by
 *       reason/action fields rather than extending ChildLockState.
 */
struct DecisionResult
{
    ChildLockState targetClState;
    DecisionReason decisionReason;
    SafetyAction safetyAction;
    bool stateChanged;
};

/**
 * @brief Determine the target child lock state from validated inputs.
 * @param input Validated decision inputs from F-01 and current state context.
 * @return DecisionResult containing target state, reason, and safety action.
 * @req_id UC-1, UC-2, UC-3
 * @traceability FG-01, F-02, SwDD 5.1.2, SwDD 5.1.3, Failure_Mode UC-1..3
 * @asil specification not explicit in repository documents
 * @note Priority order: Crash > Fault > Driver Input > Auto Lock.
 * @note Unlock requests while driving above 3 km/h are inhibited.
 */
DecisionResult DecideChildLockState(const DecisionInput& input);

}  // namespace child_lock

#endif  // CHILD_LOCK_STATE_DECISION_HPP
