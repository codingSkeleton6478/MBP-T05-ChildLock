#include "child_lock_state_decision.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace
{

using child_lock::ChildLockState;
using child_lock::DecisionInput;
using child_lock::DecisionReason;
using child_lock::DecisionResult;
using child_lock::DriverRequest;
using child_lock::FaultFlags;
using child_lock::SafetyAction;

bool Check(const bool condition, const std::string_view testName)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    return true;
}

DecisionInput MakeBaseInput()
{
    return DecisionInput{
        DriverRequest::None,
        0.0F,
        false,
        ChildLockState::Off,
        FaultFlags{false, false, false},
        false};
}

bool ExpectResult(const DecisionResult& result,
                  const ChildLockState state,
                  const DecisionReason reason,
                  const SafetyAction action,
                  const bool stateChanged,
                  const std::string_view testName)
{
    return Check(result.targetClState == state, testName) &&
           Check(result.decisionReason == reason, testName) &&
           Check(result.safetyAction == action, testName) &&
           Check(result.stateChanged == stateChanged, testName);
}

bool TestCrashHasHighestPriority()
{
    DecisionInput input = MakeBaseInput();
    input.validCrashSignal = true;
    input.validSwitchInput = DriverRequest::RequestOn;
    input.faultFlags = FaultFlags{true, true, true};
    input.autoLockConditionMet = true;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::Off,
                        DecisionReason::CrashDetected,
                        SafetyAction::EnterEmergencyReleased,
                        false,
                        "CrashHasHighestPriority");
}

bool TestFaultHasPriorityOverDriverInput()
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = ChildLockState::Off;
    input.validSwitchInput = DriverRequest::RequestOff;
    input.faultFlags.sensorFault = true;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::On,
                        DecisionReason::FaultDetected,
                        SafetyAction::EnterSafeLocked,
                        true,
                        "FaultHasPriorityOverDriverInput");
}

bool TestDriverUnlockIsBlockedWhileMoving()
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = ChildLockState::On;
    input.validSwitchInput = DriverRequest::RequestOff;
    input.vehicleSpeedKmh = 3.1F;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::On,
                        DecisionReason::UnlockInhibitedWhileMoving,
                        SafetyAction::InhibitUnlock,
                        false,
                        "DriverUnlockIsBlockedWhileMoving");
}

bool TestDriverUnlockAllowedAtThreshold()
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = ChildLockState::On;
    input.validSwitchInput = DriverRequest::RequestOff;
    input.vehicleSpeedKmh = 3.0F;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::Off,
                        DecisionReason::DriverUnlockRequest,
                        SafetyAction::None,
                        true,
                        "DriverUnlockAllowedAtThreshold");
}

bool TestDriverOnRequest()
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = ChildLockState::Off;
    input.validSwitchInput = DriverRequest::RequestOn;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::On,
                        DecisionReason::DriverLockRequest,
                        SafetyAction::None,
                        true,
                        "DriverOnRequest");
}

bool TestAutoLock()
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = ChildLockState::Off;
    input.autoLockConditionMet = true;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::On,
                        DecisionReason::AutoLockConditionMet,
                        SafetyAction::None,
                        true,
                        "AutoLock");
}

bool TestNoConditionKeepsState()
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = ChildLockState::On;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::On,
                        DecisionReason::NoStateChange,
                        SafetyAction::None,
                        false,
                        "NoConditionKeepsState");
}

bool TestSpeedFaultUsesSafeLocked()
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = ChildLockState::Off;
    input.faultFlags.speedSignalFault = true;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::On,
                        DecisionReason::FaultDetected,
                        SafetyAction::EnterSafeLocked,
                        true,
                        "SpeedFaultUsesSafeLocked");
}

bool TestSensorFaultUsesSafeLocked()
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = ChildLockState::Off;
    input.faultFlags.sensorFault = true;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::On,
                        DecisionReason::FaultDetected,
                        SafetyAction::EnterSafeLocked,
                        true,
                        "SensorFaultUsesSafeLocked");
}

bool TestStateInconsistencyUsesSafeLocked()
{
    DecisionInput input = MakeBaseInput();
    input.currentClState = ChildLockState::Off;
    input.faultFlags.stateInconsistency = true;

    return ExpectResult(child_lock::DecideChildLockState(input),
                        ChildLockState::On,
                        DecisionReason::FaultDetected,
                        SafetyAction::EnterSafeLocked,
                        true,
                        "StateInconsistencyUsesSafeLocked");
}

}  // namespace

int main()
{
    const bool ok = TestCrashHasHighestPriority() &&
                    TestFaultHasPriorityOverDriverInput() &&
                    TestDriverUnlockIsBlockedWhileMoving() &&
                    TestDriverUnlockAllowedAtThreshold() &&
                    TestDriverOnRequest() &&
                    TestAutoLock() &&
                    TestNoConditionKeepsState() &&
                    TestSpeedFaultUsesSafeLocked() &&
                    TestSensorFaultUsesSafeLocked() &&
                    TestStateInconsistencyUsesSafeLocked();

    if (!ok)
    {
        return EXIT_FAILURE;
    }

    std::cout << "All child_lock_state_decision tests passed.\n";
    return EXIT_SUCCESS;
}
