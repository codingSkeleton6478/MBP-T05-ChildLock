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


