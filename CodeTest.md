# Code-Level Test Cases (CodeTest.md)

## 1. 개요 (Overview)
본 문서는 `MBP-T05-ChildLock` 프로젝트의 코드 수준(Code-Level) 단위 테스트 프레임워크와 구현된 테스트 케이스의 상세 내역, 그리고 실행 결과를 정의합니다. 
F-01부터 F-10까지 모든 주요 모듈에 대해 GoogleTest(GTest) 프레임워크를 기반으로 작성되었습니다.

## 2. 테스트 환경 및 프레임워크 (Environment & Framework)
- **Framework**: [GoogleTest (GTest) v1.17.0](https://github.com/google/googletest)
- **Build System**: CMake (FetchContent) / CTest
- **Compiler**: GCC (MinGW-w64) / Clang
- **Coverage Target**: Statement, Branch, MC/DC 100% 지향 

## 3. 테스트 케이스 구성 (Test Case Structure)

전체 140개의 테스트 케이스가 다음 10개의 단위 테스트 스위트에 나뉘어 구현되어 있습니다:

| File Name | Target Module | Description |
|---|---|---|
| `test_F01_InputMonitorAndValidator.cpp` | F-01 Input Monitor and Validator | 입력 데이터의 유효성, 범위, 센서 상태 및 메시지 무결성 검증 |
| `test_F02_ChildLockStateDecision.cpp`   | F-02 Child Lock State Decision   | 차일드락 논리적 상태 전이, 사용자 입력 및 Override 조건 테스트 |
| `test_F03_DoorEcuCommandHandler.cpp`    | F-03 Door ECU Command Handler    | Door ECU로 전달되는 Lock/Unlock 커맨드 생성 로직 검증 |
| `test_F04_RearDoorOpenBlockHandler.cpp` | F-04 Rear Door Open Block Handler| 후석 도어 강제 개방 차단 로직 (속도/기어 연동) |
| `test_F05_StatePersistenceManager.cpp`  | F-05 State Persistence Manager   | 점화 Off 시 상태 저장 및 복원 안정성 검증 |
| `test_F06_HmiAndEventLogger.cpp`        | F-06 HMI and Event Logger        | 경고음, 디스플레이 메시지, 이벤트 로거 호출 조건 검증 |
| `test_F07_RearRiskEvaluation.cpp`       | F-07 Rear Risk Evaluation        | 후방 위험 객체 거리/상대속도 연산, 결함 검출(Fault Injection), Hysteresis 연산 |
| `test_F08_RearRiskProtectionController.cpp`| F-08 Rear Risk Protection Controller | 위험 감지 시 차일드락 자동 잠금, 센서 고장 시 Fail-Safe 모드 전환 로직 |
| `test_F09_RearSeatOccupancyAlert.cpp`   | F-09 Rear Seat Occupancy Alert   | 후방 좌석 승객 감지 및 경고 출력 로직 |
| `test_F10_IgnitionOffStatusAlert.cpp`   | F-10 Ignition Off Status Alert   | 시동 Off 하차 시 하차 위험 및 차일드락 상태 최종 고지 로직 |

### 3.1. 주요 테스트 시나리오 타겟 (Scenario Targets)
1. **Positive & Hysteresis Scenarios**: 정상적인 임계치 테스트 및 상태 유지 검증
2. **Boundary Scenarios**: 임계값 경계 조건(Boundary Conditions) 검증
3. **Fault Injection (고장 주입) Scenarios**: 센서 오류(`sensorHealth = false`), 낮은 신뢰도, Out-of-Range 데이터에 대한 강건성 및 Fail-safe 동작 확인
4. **Robustness**: 널(Null) 포인터 입력 시 방어 로직 검증

## 4. 테스트 실행 결과 (Execution Results)

코드 수준의 모든 테스트는 다음과 같이 CTest를 통해 성공적으로 수행되었습니다.

- **Total Test Cases**: 140
- **Passed**: 140 (100% Success)
- **Failed**: 0
- **Execution Log (Summary)**: 
```text
Test project C:/GIThub coding/1. F-07,F-08 개발/MBP-T05-ChildLock/build
    Start  1: test_F01_InputMonitorAndValidator
1/10 Test  #1: test_F01_InputMonitorAndValidator ........   Passed  
    Start  2: test_F02_ChildLockStateDecision
2/10 Test  #2: test_F02_ChildLockStateDecision ..........   Passed  
...
10/10 Test #10: test_F10_IgnitionOffStatusAlert .........   Passed

100% tests passed, 0 tests failed out of 10
(140 individual test cases matched and passed)
```

## 5. 빌드 및 테스트 재현 방법 (How to Build & Run)
로컬 환경에서 코드 수준 테스트를 다시 실행하려면 다음 명령어를 사용하십시오:
```bash
# 1. 빌드 폴더 생성 및 CMake 설정 (GoogleTest Fetch 포함)
cmake -B build -S .

# 2. 프로젝트 소스 및 테스트 바이너리 빌드
cmake --build build

# 3. 테스트 실행 (CTest)
cd build
ctest --output-on-failure
```

