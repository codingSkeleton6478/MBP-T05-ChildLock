# 🧪 기능 테스트 케이스 (Functional Test Case: ReqTest.md)

이 문서는 `Software_Requirement_Specification.md` 및 설계 다이어그램을 기반으로 작성된 기능 테스트 케이스를 관리합니다.

## 📋 테스트 개요
- **대상 시스템**: 전자식 차일드 락 시스템 (Electronic Child Lock System)
- **테스트 목적**: 요구사항 명세서(SRS)에 정의된 모든 기능(UC-1 ~ UC-7) 및 예외 상황에 대한 검증
- **참조 문서**:
    - [Software_Requirement_Specification.md](Software_Requirement_Specification.md)
    - [Software_Detailed_Design.md](Software_Detailed_Design.md)

## 🗂️ 테스트 케이스 목록 (System Test Cases)

SRS 요구사항(UC-1 ~ UC-7)과 ISO 26262/ASPICE 표준을 준수하여 설계된 테스트 케이스 요약입니다. 상세 내용은 첨부된 엑셀 파일을 참조하십시오.

| ID | 요구사항 ID | 테스트 항목 | 적용 기법 | 우선순위 |
| :--- | :--- | :--- | :--- | :--- |
| TC-UC1-001 | UC-1 | 정차 중 수동 스위치 제어 | Equivalence Partitioning | High |
| TC-UC1-003 | UC-1 | 주행 중 해제 차단 (Safety) | Boundary Value Analysis | High |
| TC-UC2-001 | UC-2 | 속도 임계치 초과 시 자동 잠금 | Boundary Value Analysis | High |
| TC-UC3-001 | UC-3 | 충돌 감지 시 비상 해제 | Decision Table / Safety | High |
| TC-UC4-001 | UC-4 | 내부 핸들 조작 차단 (Lock) | State Transition | High |
| TC-UC5-001 | UC-5 | 후방 위험 접근 시 보호 | Decision Table | High |
| TC-UC6-001 | UC-6 | 승객 점유 시 알림 | Equivalence Partitioning | Medium |
| TC-UC7-001 | UC-7 | 시동 OFF 시 상태 알림 | State Transition | Low |

---

## 📊 엑셀 결과 파일
상세 시스템 테스트 케이스 명세서는 아래 링크에서 확인할 수 있습니다.
- [System_Test_Cases.xlsx](System_Test_Cases.xlsx)

---

## 🛡️ 자동차 SW 검증 전문가 의견 (ISO 26262 & ASPICE)

### 1. 추가 테스트 기법 제안
- **결함 주입 테스트 (Fault Injection)**: 하드웨어 센서 단선, CAN 통신 타임아웃 등 비정상 상황에서 시스템이 안전 상태(Safe State)를 유지하는지 검증하는 것이 필수적입니다.
- **페어와이즈 테스트 (Pairwise)**: PICT 도구를 활용하여 다양한 차량 설정(Variant) 및 센서 조합에 대해 최소한의 테스트 세트로 최대의 커버리지를 확보해야 합니다.
- **B2B (Back-to-Back) 테스트**: 모델 기반 설계(MBD)를 사용하는 경우, Simulink 모델(MIL)과 코드(SIL) 간의 결과 일치 여부를 확인하여 코드 생성 과정의 오류를 방지해야 합니다.

### 2. 컴플라이언스 조언
- **커버리지 확보**: 양산 수준의 품질을 위해 문장(Statement), 분기(Branch), MC/DC 커버리지를 100% 달성하는 전략이 필요합니다.
- **HILS 검증**: 실차 환경과 유사한 실시간 시뮬레이션(HILS) 단계에서 통신 부하 및 타이밍 이슈를 사전에 검증하는 것이 권장됩니다.
