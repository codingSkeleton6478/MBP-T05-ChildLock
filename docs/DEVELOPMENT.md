# 🚙 차일드 락 제어 SW 개발 및 형상관리 가이드 (Development & Configuration Management Guide)

본 문서는 전자식 차일드 락 시스템의 일관된 로컬 빌드 환경 유지, GitHub Actions 기반의 CI/CD 파이프라인 연동, 그리고 ISO 26262 기능 안전 표준 준수를 위한 표준 개발 환경 설정 가이드를 제공합니다.

---

## 1. 표준 개발 환경 (Standard Environment)

팀 내 발생하는 환경 차이 문제를 원천 차단하기 위해, 모든 팀원은 아래 명시된 OS 및 컴파일러 사양을 엄격히 준수해야 합니다.

* **OS: Ubuntu 22.04 LTS (Jammy Jellyfish)**
  * **선정 사유:** GitHub Actions의 `ubuntu-22.04` 러너와 완벽하게 일치하여 로컬-서버 간 빌드 재현성을 100% 보장합니다. Windows 환경의 팀원은 WSL2(Windows Subsystem for Linux) 또는 Docker Container를 사용하여 해당 환경을 구축할 것을 강력히 권장합니다.
* **Compiler: GCC / G++ GCC 14.3 또는 15.2**
  * **선정 사유:** 과제/팀 협업용 기본값은 GCC 14.3, 여유가 있으면 GCC 15.2도 후보로 두는 게 좋습니다. GCC 15부터는 C23 관련 기본 동작 변화가 있어, 과제 환경에서는 한 단계 검증된 14.x가 더 무난할 수 있습니다.
* **Python: 3.13.x**
  * **선정 사유:** 3.14가 최신 기능 버전이지만, 팀 과제 자동화/스크립트 용도라면 상대적으로 보수적으로 3.13 계열 고정이 좋아 보입니다. 현재도 3.13 유지보수 릴리스가 계속 제공되고 있습니다.

<br>

## 2. 검증 및 의존성 도구 (Verification & Dependencies)

ISO 26262에서 요구하는 정적 분석(Static Analysis)과 동적 검증(Unit Test, Coverage)을 수행하기 위한 오픈소스 기반의 툴체인입니다.

| 항목 | 도구 (Tool) | 권장 버전 | 용도 및 설정 가이드 |
| :--- | :--- | :--- | :--- |
| **Build System** | **CMake** | v3.22 이상 | 플랫폼 독립적인 빌드 스크립트 생성 (모든 빌드는 CMake 기반으로 통일) |
| **Unit Test** | **Google Test** | v1.17.0 | 단, 이 버전은 C++17 이상 요구이므로 팀 표준도 C++17로 맞추는 것이 자연스럽습니다. |
| **Static Analysis** | **Cppcheck** | v2.20.0 이상 | 코드 결함 및 코딩 규칙 검사 (**MISRA C:2012 / AUTOSAR C++14 룰셋** 활성화 필수) |
| **Complexity** | **Lizard** | v1.21.2 | 함수별 순환 복잡도(Cyclomatic Complexity) 측정 (제한값 10~15 이하 유지) |
| **Coverage** | **Gcov / Lcov** | GCC 14.3과 동일 계열 | 단위 테스트 기반 구문(Statement) 및 분기(Branch) 커버리지 리포트 생성 |
| **Metrics** | **Cloc** | v1.96 | 물리적 코드 라인 수(LOC) 및 주석 비율 측정 |

<br>

## 3. 환경 구축 가이드 (Installation)

프로젝트 루트 디렉토리에서 아래 명령어를 순서대로 실행하여 환경을 구축합니다. (WSL2 또는 Native Ubuntu 환경 기준)

### Step 1. 기본 빌드 툴체인 및 검증 도구 설치
```bash
# 시스템 패키지 목록 업데이트
sudo apt-get update

# C/C++ 컴파일러, CMake 및 필수 분석 도구 일괄 설치
sudo apt-get install -y build-essential cmake cppcheck cloc lcov python3-pip python3-venv
```

### Step 2. Python 기반 분석 도구 설치
시스템 파이썬 패키지와의 충돌을 막기 위해 가상환경(Virtual Environment) 설정을 권장합니다.
```bash
# 가상환경 생성 및 활성화
python3 -m venv .venv
source .venv/bin/activate

# Lizard 설치
pip install lizard
```

### Step 3. 빌드 및 테스트 확인 (Sanity Check)
환경 구축이 완료되면, 프로젝트의 CMake를 활용해 정상적으로 빌드되는지 확인합니다.
```bash
# 빌드 디렉토리 생성 및 컴파일
mkdir build && cd build
cmake ..
make -j$(nproc)

# 단위 테스트 실행
ctest --output-on-failure
```
