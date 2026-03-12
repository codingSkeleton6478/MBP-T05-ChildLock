#include "child_lock_state_decision.hpp"

namespace child_lock
{

namespace
{

constexpr float kUnlockInhibitSpeedKmh = 3.0F;

bool HasAnyFault(const FaultFlags& faultFlags)
{
    return faultFlags.speedSignalFault ||
           faultFlags.sensorFault ||
           faultFlags.stateInconsistency;
}

DecisionResult MakeResult(const ChildLockState targetState,
                          const DecisionReason reason,
                          const SafetyAction action,
                          const ChildLockState currentState)
{
    return DecisionResult{targetState, reason, action, targetState != currentState};
}

DecisionResult EvaluateDriverRequest(const DecisionInput& input)
{
    if (input.validSwitchInput == DriverRequest::RequestOn)
    {
        return MakeResult(ChildLockState::On,
                          DecisionReason::DriverLockRequest,
                          SafetyAction::None,
                          input.currentClState);
    }

    if (input.validSwitchInput == DriverRequest::RequestOff)
    {
        if (input.vehicleSpeedKmh <= kUnlockInhibitSpeedKmh)
        {
            return MakeResult(ChildLockState::Off,
                              DecisionReason::DriverUnlockRequest,
                              SafetyAction::None,
                              input.currentClState);
        }

        // Specification maps driving unlock inhibition to current-state hold.
        return MakeResult(input.currentClState,
                          DecisionReason::UnlockInhibitedWhileMoving,
                          SafetyAction::InhibitUnlock,
                          input.currentClState);
    }

    return MakeResult(input.currentClState,
                      DecisionReason::NoStateChange,
                      SafetyAction::None,
                      input.currentClState);
}

}  // namespace

DecisionResult DecideChildLockState(const DecisionInput& input)
{
    if (input.validCrashSignal)
    {
        return MakeResult(ChildLockState::Off,
                          DecisionReason::CrashDetected,
                          SafetyAction::EnterEmergencyReleased,
                          input.currentClState);
    }

    if (HasAnyFault(input.faultFlags))
    {
        return MakeResult(ChildLockState::On,
                          DecisionReason::FaultDetected,
                          SafetyAction::EnterSafeLocked,
                          input.currentClState);
    }

    if (input.validSwitchInput != DriverRequest::None)
    {
        return EvaluateDriverRequest(input);
    }

    if (input.autoLockConditionMet)
    {
        return MakeResult(ChildLockState::On,
                          DecisionReason::AutoLockConditionMet,
                          SafetyAction::None,
                          input.currentClState);
    }

    return MakeResult(input.currentClState,
                      DecisionReason::NoStateChange,
                      SafetyAction::None,
                      input.currentClState);
}

}  // namespace child_lock
