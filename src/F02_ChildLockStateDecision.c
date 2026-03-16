#include "F02_ChildLockStateDecision.h"

static const float kUnlockInhibitSpeedKmh = 3.0F;

static bool HasAnyFault(const FaultFlags* faultFlags)
{
    return faultFlags->speedSignalFault ||
           faultFlags->sensorFault ||
           faultFlags->stateInconsistency;
}

static DecisionResult MakeResult(const ChildLockState targetState,
                                  const DecisionReason reason,
                                  const SafetyAction action,
                                  const ChildLockState currentState)
{
    DecisionResult result;
    result.targetClState   = targetState;
    result.decisionReason  = reason;
    result.safetyAction    = action;
    result.stateChanged    = (targetState != currentState);
    return result;
}

// cppcheck-suppress unusedFunction
DecisionResult DecideChildLockState(const DecisionInput* input)
{
    if (input->validCrashSignal)
    {
        return MakeResult(CL_OFF,
                          DECISION_CRASH_DETECTED,
                          SAFETY_ENTER_EMERGENCY_RELEASED,
                          input->currentClState);
    }

    if (HasAnyFault(&input->faultFlags))
    {
        return MakeResult(CL_ON,
                          DECISION_FAULT_DETECTED,
                          SAFETY_ENTER_SAFE_LOCKED,
                          input->currentClState);
    }

    if (input->validSwitchInput == DRIVER_REQ_ON)
    {
        return MakeResult(CL_ON,
                          DECISION_DRIVER_LOCK_REQUEST,
                          SAFETY_NONE,
                          input->currentClState);
    }

    if (input->validSwitchInput == DRIVER_REQ_OFF)
    {
        if (input->vehicleSpeedKmh <= kUnlockInhibitSpeedKmh)
        {
            return MakeResult(CL_OFF,
                              DECISION_DRIVER_UNLOCK_REQUEST,
                              SAFETY_NONE,
                              input->currentClState);
        }

        return MakeResult(input->currentClState,
                          DECISION_UNLOCK_INHIBITED_WHILE_MOVING,
                          SAFETY_INHIBIT_UNLOCK,
                          input->currentClState);
    }

    if (input->autoLockConditionMet)
    {
        return MakeResult(CL_ON,
                          DECISION_AUTO_LOCK_CONDITION_MET,
                          SAFETY_NONE,
                          input->currentClState);
    }

    return MakeResult(input->currentClState,
                      DECISION_NO_STATE_CHANGE,
                      SAFETY_NONE,
                      input->currentClState);
}
