# 📱 Electronic Child Lock System (MBP-T05)

<div align="center">
  <img src="docs/diagram/usecase/usecase_normal.png" width="600" alt="Project Overview">
  <h3>MOBIUS Bootcamp ISO 26262 SW Simulation Project</h3>
  <p><b>Team Jeoncha (전차)</b></p>
</div>

---

## 📝 Project Overview

This project involves the development of a **Functionally Safe Electronic Child Lock System** in compliance with **ISO 26262** standards. The system focuses on preventing accidents by intelligently controlling rear door locks based on vehicle speed, collision detection, and external hazards.

- **Objective**: Implement a robust Child Lock system with high reliability and safety.
- **Key Focus**: ISO 26262 Functional Safety, MISRA C compliance, and TDD-based development.

---

## 🚀 Key Features

The system is designed around 7 core Use Cases, categorized into two major Feature Groups:

### **FG-01: Door Lock / Unlock Control**

- **UC-1: Manual Control** – Electronic ON/OFF control by the driver.
- **UC-2: Auto Activation** – Automatic locking above 3km/h.
- **UC-3: Emergency Release** – Immediate unlocking upon collision detection.
- **UC-4: Opening Block** – Prevents door opening from inside when CL is ON.

### **FG-02: Exit Safety Assist**

- **UC-5: Hazard Detection** – Prevents unlocking if a rear-approaching object is detected.
- **UC-6: Occupancy Alert** – Alerts the driver if passengers are in the back upon departure.
- **UC-7: Status Notification** – Reminds the driver of CL status when the ignition is turned OFF.

---

## 🛠 Technology Stack

- **Language**: C11 / C++17
- **Standard**: MISRA C:2012 / ISO 26262 / ASPICE
- **Environment**: Ubuntu 22.04 LTS, GCC 14.3/15.2
- **Build System**: CMake v3.22+
- **Verification Tools**:
  - **Unit Test**: Google Test v1.17.0
  - **Static Analysis**: Cppcheck (MISRA Ruleset)
  - **Metrics**: Lizard (Complexity), Cloc (LOC)
  - **Coverage**: Gcov / Lcov (Statement/Branch Coverage)

---

## ✅ Project Tasks

- [x] Research project details and Wiki files <!-- id: 0 -->
  - [x] Read SRS and Development docs <!-- id: 1 -->
  - [x] Identify all relevant documentation files for links <!-- id: 2 -->
- [x] Create Implementation Plan <!-- id: 3 -->
- [x] Update README.md <!-- id: 4 -->
  - [x] Add project overview and description <!-- id: 5 -->
  - [x] Add links to documentation files <!-- id: 6 -->
  - [x] Format as a professional portfolio <!-- id: 7 -->
- [x] Refine Portfolio Aesthetics <!-- id: 9 -->
  - [x] Add technical diagram gallery <!-- id: 10 -->
  - [x] Polish layout and spacing <!-- id: 11 -->
- [x] Verify README.md <!-- id: 8 -->

---

## 📂 Documentation (Wiki Home)
Click the links below to explore the detailed project documentation:

| Document | Description |
| :--- | :--- |
| [📜 SRS](docs/Software_Requirement_Specification.md) | Software Requirement Specification (Use Cases & Scenarios) |
| [📘 SwDD](docs/Software_Detailed_Design.md) | Software Detailed Design (Architecture & Flowcharts) |
| [🚨 Failure Mode](docs/Failure_Mode.md) | FMEA/HARA Analysis & Safety Design Policies |
| [🚙 Development Guide](docs/DEVELOPMENT.md) | Environment Setup, Build, and Verification Guide |
| [🔗 GitHub Policy](docs/GITHUB.md) | Branching, Commit, and CI/CD (GitHub Actions) Policies |
| [🤖 AI Guidelines](docs/AGENTS.md) | AI-Assisted Development & Coding Standards |

---

## 📊 System Architecture & Diagrams

To ensure functional safety and clear logic flow, the system is guided by several architectural diagrams.

### **State Machine**
The core behavior of the Child Lock system is managed by a robust state machine that handles transitions between Locked, Unlocked, and Emergency states.

<div align="center">
  <img src="docs/diagram/state/childlock_state.png" width="500" alt="State Diagram">
</div>

### **Technical Logic Flow**

The two main feature groups (FG-01 & FG-02) follow strictly defined logic paths to handle sensor inputs and safety constraints.

<div align="center">
  <table>
    <tr>
      <td align="center"><b>FG-01: Door Control Logic</b></td>
      <td align="center"><b>FG-02: Exit Safety Logic</b></td>
    </tr>
    <tr>
      <td><img src="docs/diagram/flow_chart/FG-01_door_control.png" width="400" alt="FG-01 Flow Chart"></td>
      <td><img src="docs/diagram/flow_chart/FG-02_exit_safety.png" width="400" alt="FG-02 Flow Chart"></td>
    </tr>
  </table>
</div>

---

## 📦 How to Build & Run

```bash
# Build the project
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# Run Unit Tests
ctest --output-on-failure
```

---

## 👥 Team Jeoncha (전차)

- **Team Leader**: 박기준
- **Members**: 박기준, 김도균, 김이안, 이한결, 이승욱, 박찬석
