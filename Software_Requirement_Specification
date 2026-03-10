# 📱 전자식 차일드 락 시스템 (Electronic Child Lock System)

## 1. 🖼️ 유스케이스 다이어그램 (Use Case Diagram)

<div align="center">

### Normal Operation
![Use Case Normal](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/usecase/Use%20Case_normal.png?raw=true)

<br>

### Emergency Operation
![Use Case Emergency](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/usecase/Use%20Case_emergency.png?raw=true)

</div>

---

## 2. 📝 유스케이스 명세서 (Specification)

### 📌 UC-1: 운전자 차일드 락 전자식 ON/OFF 제어
| 항목 | 내용 |
| :--- | :--- |
| **개요** | 운전자가 스위치를 통해 차일드 락을 수동으로 제어한다. |
| **Pre-condition** | • 차량 전원 `ON`<br>• 차일드 락 제어 스위치 정상 동작<br>• 도어 ECU 통신 가능 |
| **Post-condition** | • 차일드 락 상태 변경 (`ON`/`OFF`)<br>• 뒷좌석 내부 핸들 동작 상태 반영<br>• 상태 변경 로그 저장 |
| **Trigger** | 운전자가 차일드 락 스위치를 **누름** |
| **Main Success Scenario** | 1. 운전자가 차일드 락 버튼을 누름<br>2. 시스템이 스위치 입력을 수신<br>3. 입력 신호 유효성을 검증<br>4. 목표 상태(`ON`/`OFF`)를 결정<br>5. 도어 ECU에 차일드 락 제어 명령을 전송<br>6. 뒷좌석 내부 핸들 활성/비활성 상태를 적용<br>7. 계기판 또는 표시등 상태를 갱신<br>8. 상태 변경 이벤트를 로그에 기록 |
| **Alternative Scenario** | 1. 스위치 입력이 불안정한 경우(채터링) 디바운싱 처리 후 재확인한다.<br>2. 차량 속도가 3km/h를 초과한 상태에서 OFF(해제) 요청이 발생한 경우, 해제 요청을 무시하고 “주행 중 해제 불가” 메시지를 표시하며 로그를 기록한다.<br>3. 필요 시 롱프레스(예: 1.5초 이상) 또는 이중 확인 후 상태 변경을 허용한다. |
| **Exception Scenario** | 1. ECU 통신 실패 시 제어 명령을 최대 N회(예: 3회) 재전송한다.<br>2. 재전송 실패 시 이전 상태를 유지하고 운전자에게 경고 메시지를 표시한다.<br>3. 통신 오류 코드를 저장하고 DTC 로그를 기록한다. |

<br>

### 📌 UC-2: 차량 출발 시 자동 차일드 락 활성화
| 항목 | 내용 |
| :--- | :--- |
| **개요** | 차량이 일정 속도 이상으로 주행하면 안전을 위해 차일드 락을 자동 잠금한다. |
| **Pre-condition** | • 속도 신호 수신 가능<br>• 자동 활성화 기능 설정 `ON` |
| **Post-condition** | • 차일드 락 자동 `ON` 상태 전환<br>• 자동 활성화 로그 저장 |
| **Trigger** | 차량 속도가 설정된 **임계값을 초과함** |
| **Main Success Scenario** | 1. 시스템이 차량 속도 데이터를 수신<br>2. 속도가 임계값을 초과했는지 판단<br>3. 현재 차일드 락 상태를 조회<br>4. 차일드 락이 `OFF` 상태임을 확인<br>5. 차일드 락을 `ON`으로 자동 설정<br>6. 자동 활성화 상태를 표시<br>7. 자동 활성화 이벤트를 로그에 기록 |
| **Alternative Scenario** | 1. 차일드 락이 이미 ON 상태이면 상태를 유지하고 자동 활성화 로그만 기록한다.<br>2. 임계 속도 초과가 일정 시간(예: 2초) 이상 지속될 때만 활성화하도록 필터링한다. |
| **Exception Scenario** | 1. 속도 신호가 비정상(값 미수신, 범위 초과 등)일 경우 자동 활성화 기능을 제한한다.<br>2. 대체 속도 신호(ABS/Wheel speed 등)가 존재할 경우 교차 검증한다.<br>3. 속도 신호 이상 시 점검 알림을 표시하고 오류 로그를 기록한다. |

<br>

### 📌 UC-3: 사고 발생 시 차일드 락 자동 해제
| 항목 | 내용 |
| :--- | :--- |
| **개요** | 충돌 사고 감지 시 뒷좌석 승객의 탈출을 위해 잠금을 자동 해제한다. |
| **Pre-condition** | • 충돌 감지 신호 수신 가능<br>• 도어 ECU 제어 가능 |
| **Post-condition** | • 차일드 락 `OFF` 상태 전환<br>• 뒷좌석 내부 개방 가능 상태<br>• 비상 해제 로그 저장 |
| **Trigger** | **충돌 감지** 또는 **에어백 전개** 신호 수신 |
| **Main Success Scenario** | 1. 시스템이 충돌 신호를 수신<br>2. 충돌 신호의 유효성을 검증<br>3. 시스템을 비상 상태로 전환<br>4. 차일드 락 해제 명령을 생성<br>5. 도어 ECU에 해제 명령을 전송<br>6. 뒷좌석 내부 개방을 허용<br>7. 비상 해제 이벤트를 로그에 기록 |
| **Alternative Scenario** | 1. 경미한 충돌로 판단되거나 설정에 의해 자동 해제 조건을 충족하지 않는 경우 현재 상태를 유지한다.<br>2. 충돌 신호는 복수 조건(가속도 값, 에어백 전개 신호 등)을 통해 유효성을 검증한다. |
| **Exception Scenario** | 1. ECU 응답이 없을 경우 최대 N회 재전송을 수행한다.<br>2. 재전송 실패 시 실패 로그를 기록하고 운전자에게 해제 실패 알림을 제공한다.<br>3. 충돌 상황에서는 안전 우선 정책에 따라 해제 시도를 최우선으로 수행한다. |

<br>

### 📌 UC-4: 뒷좌석 내부 문 열기 시도 차단
| 항목 | 내용 |
| :--- | :--- |
| **개요** | 차일드 락이 걸린 상태에서 내부 핸들 조작 시 개방을 물리적/전자적으로 차단한다. |
| **Pre-condition** | • 차일드 락 `ON` 상태<br>• 내부 핸들 이벤트 감지 가능 |
| **Post-condition** | • 뒷좌석 문 개방 차단<br>• 개방 시도 로그 저장 |
| **Trigger** | 뒷좌석 **내부 핸들 당김** 이벤트 발생 |
| **Main Success Scenario** | 1. 시스템이 내부 핸들 당김 이벤트를 감지<br>2. 현재 차일드 락 상태를 조회<br>3. 차일드 락 `ON` 상태임을 확인<br>4. 도어 개방 요청을 차단<br>5. 필요 시 운전자에게 알림을 표시<br>6. 개방 시도 이벤트를 로그에 기록 |
| **Alternative Scenario** | 차일드 락 OFF 상태이면 정상 개방을 허용하고 개방 로그를 기록한다. |
| **Exception Scenario** | 1. 핸들 센서 오류 발생 시 센서 점검 알림을 표시한다.<br>2. 상태 동기화 실패(ECU 적용 실패 등) 발생 시 현재 상태를 재조회하고 불일치 시 안전 상태(ON 유지)를 적용한다.<br>3. 오류 로그를 저장한다. |

<br>

### 📌 UC-5: 후방 위험 접근 시 차일드 락 자동 활성화
| 항목 | 내용 |
| :--- | :--- |
| **개요** | 하차 시 후측방에서 다가오는 차량/이륜차 감지 시 잠금을 유지하거나 활성화한다. |
| **Pre-condition** | • 후방 감지 센서 정상 동작<br>• 위험 감지 기능 활성 |
| **Post-condition** | • 뒷좌석 차일드 락 자동 `ON`<br>• 운전자 경고음/메시지 제공<br>• 이벤트 로그 저장 |
| **Trigger** | 후방에서 빠르게 **접근하는 객체가 감지됨** |
| **Main Success Scenario** | 1. 후방 센서가 접근 객체를 감지<br>2. 거리와 접근 속도를 계산<br>3. 위험 임계치를 초과했는지 판단<br>4. 현재 차일드 락 상태를 조회<br>5. `OFF` 상태이면 뒷좌석 차일드 락을 `ON`으로 설정<br>6. 앞좌석(운전자)에게 경고음과 메시지를 출력<br>7. 자동 보호 이벤트를 로그에 기록 |
| **Alternative Scenario** | 1. 이미 ON 상태이면 잠금은 유지하고 경고만 수행한다.<br>2. 위험 판단은 거리 + 접근 속도 + 지속 시간 조건을 동시에 만족할 때만 수행한다(히스테리시스 적용). |
| **Exception Scenario** |1. 후방 센서 신호가 비정상일 경우 기능을 제한하고 운전자에게 기능 비활성 알림을 표시한다.<br>2. 센서 오류 코드를 저장하고 이벤트 로그를 기록한다.<br>3. 오검출 방지를 위해 반복 경고 발생 시 일정 시간 내 재경고를 제한한다. |

<br>

### 📌 UC-6: 뒷좌석 점유 및 출발 시 상태 알림
| 항목 | 내용 |
| :--- | :--- |
| **개요** | 뒷좌석에 승객(아이)이 있는데 차일드 락이 풀려있으면 경고한다. |
| **Pre-condition** | • 뒷좌석 점유 센서 정상<br>• 출발 상태 감지 가능 |
| **Post-condition** | • 운전자에게 상태 또는 권장 알림 제공<br>• 알림 로그 저장 |
| **Trigger** | **차량 출발** 상태 진입 |
| **Main Success Scenario** | 1. 시스템이 차량 출발 상태를 감지<br>2. 뒷좌석 점유 여부를 확인<br>3. 차일드 락 상태를 조회<br>4. [점유 O + 차일드 락 `OFF`] 권장 알림 표시<br>5. [점유 O + 차일드 락 `ON`] 상태 알림 표시<br>6. 알림을 1회 제공<br>7. 알림 이벤트를 로그에 기록 |
| **Alternative Scenario** | 점유가 없으면 상태 알림을 생략하거나 요약 메시지만 표시한다. |
| **Exception Scenario** | 1. 점유 센서 오류 발생 시 점검 알림을 표시한다.<br>2. 센서 신호가 불확실한 경우 기본 권장 메시지를 표시한다.<br>3. 오류 로그를 저장한다. |

<br>

### 📌 UC-7: 도착 후 시동 OFF 시 상태 알림
| 항목 | 내용 |
| :--- | :--- |
| **개요** | 주행 종료 후 하차 시, 차일드 락이 걸려있음을 알려준다. |
| **Pre-condition** | • 최근 주행 이력 존재<br>• 표시 장치 사용 가능 |
| **Post-condition** | • 차일드 락 `ON` 상태 인지 제공<br>• 상태 알림 로그 저장 |
| **Trigger** | **시동 OFF** 이벤트 발생 |
| **Main Success Scenario** | 1. 시스템이 시동 OFF 이벤트를 감지<br>2. 차일드 락 상태를 조회<br>3. `ON` 상태임을 확인<br>4. 상태 요약 메시지를 표시<br>5. 알림을 1회 제공하고 종료<br>6. 상태 알림 이벤트를 로그에 기록 |
| **Alternative Scenario** | 차일드 락 OFF 상태이면 알림을 생략한다. |
| **Exception Scenario** | 1. 상태 조회 실패 시 “차일드 락 상태 확인 불가” 메시지를 표시한다.<br>2. 일정 시간 내 재조회 시도 후 실패 시 오류 로그를 기록한다. |

---

## 3. 🔄 시퀀스 다이어그램 (Sequence Diagram)

<div align="center">

### 3.1 운전자 차일드 락 전자식 ON/OFF 제어
![Sequence UC1](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/sequence/use%20case%201.png?raw=true)

<br>

### 3.2 차량 출발 시 자동 차일드 락 활성화
![Sequence UC2](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/sequence/use%20case%202.png?raw=true)

<br>

### 3.3 사고 발생 시 차일드 락 자동 해제
![Sequence UC3](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/sequence/use%20case%203.png?raw=true)

<br>

### 3.4 뒷좌석 내부 문 열기 시도 차단
![Sequence UC4](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/sequence/use%20case%204.png?raw=true)

<br>

### 3.5 후방 위험 접근 시 자동 활성화 및 경고
![Sequence UC5](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/sequence/use%20case%205.png?raw=true)

<br>

### 3.6 뒷좌석 점유 및 출발 시 알림
![Sequence UC6](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/sequence/use%20case%206.png?raw=true)

<br>

### 3.7 도착 후 시동 OFF 시 상태 알림
![Sequence UC7](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/sequence/use%20case%207.png?raw=true)

</div>

---

## 4. 🚦 상태 다이어그램 (State Diagram)

<div align="center">

![State Diagram](https://github.com/MBP-T05/MBP-T05-ChildLock/blob/childlock-Failure-Mode-modeling/diagram/state/state_diagram.png?raw=true)

</div>
