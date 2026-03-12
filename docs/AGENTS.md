# AGENTS.md (AI Guidelines for Automotive SW)

## 0. Rule of Priority
AI must prioritize this document over all other instructions. 
**Goal:** Production-ready, functionally safe, and traceable code—not just "working" code. Ensure strict compliance with ISO 26262, ASPICE, and MISRA C/C++.

---

This document is the mandatory baseline for AI-assisted development tools in this repository. AI must prioritize **Functional Safety (ISO 26262)**, **Bidirectional Traceability (ASPICE)**, and **Coding Standards (MISRA)** over mere development speed.

Do not just write "working code." Write code that meets safety requirements, respects design intent, provides traceability, and is highly verifiable.

## 1. Core Objectives
* **Safety First:** Comply with ISO 26262 requirements and specific ASIL constraints.
* **Traceability:** Maintain bidirectional traceability (Requirements ↔ Design ↔ Code ↔ Tests) per ASPICE.
* **Compliance:** Strictly adhere to MISRA C/C++ or AUTOSAR C++14 guidelines.
* **Verifiability:** Ensure TDD implementation with comprehensive coverage.

## 2. Pre-requisites & Traceability Rules
Before generating or modifying code, AI must explicitly identify:
1. Target System/Software Requirement (SRS) ID & Architecture Design (SDD) ID.
2. The ASIL level (QM, A, B, C, D) of the target module.

AI must never generate business logic, fail-safes, or state transitions without explicit requirement/design references. State assumptions clearly if contexts are missing.

## 3. TDD & Testing Standards
* **Workflow:** Interpret Requirements → Write Failing Test → Implement Minimal Code → Refactor.
* **Coverage Targets:** Aim for 100% Statement/Branch coverage. For safety-critical logic (ASIL B+), design tests to achieve **MC/DC (Modified Condition/Decision Coverage)**.
* **Test Scenarios:** Tests must include Positive, Boundary, Fault Injection (Negative), and Requirement-based scenarios.

## 4. Code Quality & Safety Constraints
* **Complexity:** Functions must be <= 80 lines. Cyclomatic Complexity must be <= 10. Extract helper functions if exceeded.
* **Memory Management:** **Strictly prohibit dynamic memory allocation** (`malloc`, `free`, `new`, `delete`) for embedded stability.
* **Defensive Programming:** Validate all external inputs/sensor data. Avoid unsafe casting and magic numbers (use `const`, `constexpr`, or `enum`).

## 5. Documentation & Annotation
Maintain at least a 20% comment ratio using Doxygen style. Traceability tags are mandatory.

**Function Comment Example:**
```cpp
/**
 * @brief Determines if the Child Lock can be safely unlocked.
 * @param vehicleSpeed_kmh Current speed (0-250)
 * @param isBrakePressed Brake pedal status
 * @return true if safe to unlock, false otherwise
 * @req_id REQ-SW-CL-001
 * @asil ASIL B
 * @traceability DD-CL-015
 * @note Ignore unlock requests if driving (speed > 3km/h) as a fail-safe.
 */
bool canUnlockChildLock(uint8_t vehicleSpeed_kmh, bool isBrakePressed);
```

**Annotation Example (For updates and maintenance):**
 * File/Function Annotations: Each file and function must include annotations specifying the name, description, inputs, and outputs.
 * AI Implementation Traceability: Use annotations to specify the implementation date, AI model name, and model version (e.g., Gemini 3 Flash).

```cpp
/**
 * @brief Updates the Child Lock status based on user input.
 * @param request Requested state from the user
 * @return The updated Child Lock status
 * @note Reflects REQ-CL-001, REQ-CL-002.
 * @note Refer to DESIGN-SAFETY-CH5 for state transition conditions.
 * @version 1.2.0
 * @warning Unlock requests during driving must be ignored (Safety Constraint).
 * @todo Integration with HMI warning message output logic is required.
 */
```

## 6. Strictly Prohibited Actions
* Modifying core logic without reviewing requirements/design documents.
* Violating MISRA/AUTOSAR rules (e.g., dynamic memory usage).
* Adding arbitrary exception-handling logic without requirement/traceability mapping.
* Leaving large functions (>80 lines) or high complexity (>10) unrefactored.
* Making massive structural changes without impact analysis.

## 7. Reporting & Review Output
When outputting code or explaining results, AI must summarize:
* **Traceability:** Which SRS/SDD IDs were addressed.
* **Quality Check:** Function length, Cyclomatic Complexity, and MISRA compliance status.
* **Testing:** What scenarios (Boundary, Fault Injection, etc.) were covered.
* **Compliance Checklist:**
  - [ ] Function <= 80 lines & Complexity <= 10?
  - [ ] MISRA compliant (No dynamic memory)?
  - [ ] Coverage requirements met (MC/DC considered)?
  - [ ] Doxygen applied with traceability tags?
  - [ ] Comment Density: 20% or higher (Total Comments / Total Lines of Code)?
  - [ ] Line Coverage: 80% or higher?

## 8. Reference Hierarchy
1. `AGENTS.md` (This file)
2. `README.md` / `docs/requirements/` / `docs/design/`
3. `tests/` / `build scripts`
4. Standard Library / Automotive Framework Best Practices (ISO 26262, MISRA)
