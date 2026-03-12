# 1. GitHub 정책 문서에 추가할 사항
GitHub를 Issue / PR / Wiki / Action 용

추가하면 좋은 섹션

### 1) Branch Naming Convention

````markdown
## Branch Naming Convention

모든 작업은 Issue 기반으로 브랜치를 생성한다.

- main : 안정 브랜치
- feature/#이슈번호-기능명
- fix/#이슈번호-버그명
- docs/#이슈번호-문서명

예시:
- feature/#12-manual-childlock
- fix/#18-speed-signal-validation
- docs/#05-update-development-doc
````

### 2) Commit Message Convention
커밋 메시지 규칙 추가

````markdown
## Commit Message Convention

커밋 메시지는 아래 Prefix를 사용한다.

- feat: 새로운 기능 추가
- fix: 버그 수정
- docs: 문서 수정
- refactor: 리팩토링
- test: 테스트 코드 추가/수정
- chore: 빌드/설정 변경

예시:
- feat: add automatic child lock activation logic
- fix: block unlock command while vehicle is moving
- docs: update DEVELOPMENT.md
````

### 3) Pull Request Rules

````markdown
## Pull Request Rules

- main 브랜치 직접 push는 금지한다.
- 모든 코드 변경은 Pull Request를 통해 병합한다.
- Pull Request 생성 전 빌드와 테스트를 수행해야 한다.
- PR 설명에는 변경 목적, 주요 변경 내용, 테스트 결과를 포함한다.
- 가능하면 1인 이상 리뷰 후 병합한다.
````

### 4) Definition of Done

````markdown
## Definition of Done

아래 조건을 만족해야 작업 완료로 간주한다.

- 관련 Issue가 등록되어 있을 것
- 작업 브랜치에서 구현이 완료되었을 것
- 코드가 정상적으로 빌드될 것
- 단위 테스트가 통과할 것
- 정적 분석 결과 치명적인 오류가 없을 것
- 관련 문서가 업데이트될 것
- Pull Request 리뷰가 완료될 것
````

### 5) CI Policy

````markdown
## CI Policy

GitHub Actions는 push 및 pull request 이벤트에서 자동 실행한다.

기본 검사 항목:
- Build
- Unit Test
- Static Analysis (CppCheck)
- Coverage Measurement

CI 결과가 실패한 경우 main 브랜치에 병합하지 않는다.
````

---

# 2. DEVELOPMENT.md에 추가 가능 사항
설치후 사용법

---

### 추가 1) Build Instructions

````markdown
## 3. Build Instructions

프로젝트는 CMake 기반으로 빌드한다.

```bash
mkdir -p build
cd build
cmake ..
make
```
````

빌드 결과 실행 파일 및 테스트 바이너리는 `build` 디렉토리에 생성된다.

````markdown
---

## 추가 2) Unit Test 실행 방법

```md
## 4. Unit Test Instructions

단위 테스트는 Google Test를 사용하여 수행한다.

```bash
cd build
ctest --output-on-failure
```
````

또는 테스트 바이너리를 직접 실행할 수 있다.

```bash
./unit_tests
```

모든 테스트는 Pull Request 생성 전에 통과해야 한다.

````markdown
---

## 추가 3) Coverage 측정 방법

```md
## 5. Coverage Measurement

커버리지는 Gcov / Lcov를 사용하여 측정한다.

```bash
mkdir -p build
cd build
cmake -DENABLE_COVERAGE=ON ..
make
ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```
````

HTML 보고서는 아래 경로에서 확인할 수 있다.

```text
build/coverage_report/index.html
```

````markdown
---

## 추가 4) Static Analysis 실행 방법

```md
## 6. Static Analysis

정적 분석은 CppCheck를 사용한다.

```bash
cppcheck --enable=all --inconclusive --std=c11 --suppress=missingIncludeSystem src/
```
````

정적 분석 결과를 통해 잠재적 버그, 메모리 오류, 코딩 규칙 위반 여부를 점검한다.

````markdown
---

## 추가 5) Code Metrics 실행 방법

```md
## 7. Code Metrics

### Cyclomatic Complexity
```bash
lizard src/
```
````

### Lines of Code

```bash
cloc src test
```

Lizard는 함수 단위 복잡도를 측정하고, Cloc은 코드 라인 수를 분석한다.

````markdown
---

## 추가 6) Project Directory Structure

```md
## 8. Project Directory Structure

프로젝트는 아래 구조를 따른다.

```text
project
├── src/                # Production source code
├── include/            # Header files
├── test/               # Unit test code
├── doc/
│   └── uml_diagram/    # UML, Flowchart diagrams
├── .github/
│   └── workflows/      # GitHub Actions CI
├── DEVELOPMENT.md
├── README.md
└── CMakeLists.txt
```
````

````markdown
---

## 추가 7) Coding Guidelines

```md
## 9. Coding Guidelines

- C11 표준을 준수한다.
- 전역 변수 사용을 최소화한다.
- 매직 넘버 대신 상수를 사용한다.
- 모든 입력값은 유효성 검사를 수행한다.
- 함수는 단일 책임 원칙을 따른다.
- 예외 상황 발생 시 명확한 오류 처리 경로를 구현한다.
```
````

---

# 3. GitHub Actions CI yaml 예시
아래는 Ubuntu 22.04 환경에서

- build
- test
- cppcheck
- lizard
- cloc

까지 기본적으로 도는 예시다.

**전제:**
- 프로젝트 루트에 `CMakeLists.txt` 있음
- 테스트는 `ctest`로 실행 가능
- 소스는 `src/`, 테스트는 `test/`

**파일 위치:**
`.github/workflows/ci.yml`

**코드:**
```yaml
name: C CI

on:
  push:
    branches: [ "main", "develop", "feature/**", "fix/**", "docs/**" ]
  pull_request:
    branches: [ "main", "develop" ]

jobs:
  build-test-analyze:
    runs-on: ubuntu-22.04

    steps:
      - name: Checkout repository
        uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            build-essential \
            gcc \
            g++ \
            cmake \
            make \
            cppcheck \
            cloc \
            lcov \
            python3 \
            python3-pip
          pip3 install lizard

      - name: Configure project
        run: |
          mkdir -p build
          cd build
          cmake ..

      - name: Build project
        run: |
          cd build
          make

      - name: Run unit tests
        run: |
          cd build
          ctest --output-on-failure

      - name: Run cppcheck
        run: |
          cppcheck --enable=all --inconclusive --std=c11 \
            --suppress=missingIncludeSystem \
            src/

      - name: Run lizard
        run: |
          lizard src/

      - name: Run cloc
        run: |
          cloc src test
```

### 커버리지까지 넣고 싶을 때
`ENABLE_COVERAGE=ON` 옵션이 CMake에 준비되어 있다면 아래 job 또는 step을 더 붙일 수 있다.

```yaml
      - name: Configure project for coverage
        run: |
          rm -rf build
          mkdir -p build
          cd build
          cmake -DENABLE_COVERAGE=ON ..

      - name: Build for coverage
        run: |
          cd build
          make

      - name: Run tests for coverage
        run: |
          cd build
          ctest --output-on-failure

      - name: Generate coverage report
        run: |
          cd build
          lcov --capture --directory . --output-file coverage.info
          genhtml coverage.info --output-directory coverage_report
```
