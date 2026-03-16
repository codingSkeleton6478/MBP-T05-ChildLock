cmake_minimum_required(VERSION 3.22)

project(MBP-T05-ChildLock VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compile options: common flags for both C and C++
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wconversion -Wsign-conversion
    -Wformat=2 -Wundef -Wshadow
    -Wcast-align -Wredundant-decls
)

# C-only flags (MISRA C:2012 friendly; not valid for C++)
add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>
    $<$<COMPILE_LANGUAGE:C>:-Wmissing-prototypes>
    $<$<COMPILE_LANGUAGE:C>:-Wmissing-declarations>
)

# Coverage support
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    message(STATUS "Coverage enabled")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

# F-01: Input Monitor and Validator (C)
add_library(F01_InputMonitorAndValidator
    src/F01_InputMonitorAndValidator.c
)
target_include_directories(F01_InputMonitorAndValidator
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-02: Child Lock State Decision (C)
add_library(F02_ChildLockStateDecision
    src/F02_ChildLockStateDecision.c
)
target_include_directories(F02_ChildLockStateDecision
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-03: Door ECU Command Handler (C)
add_library(F03_DoorEcuCommandHandler
    src/F03_DoorEcuCommandHandler.c
)
target_include_directories(F03_DoorEcuCommandHandler
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-04: Rear Door Open Block Handler (C)
add_library(F04_RearDoorOpenBlockHandler
    src/F04_RearDoorOpenBlockHandler.c
)
target_include_directories(F04_RearDoorOpenBlockHandler
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-05: State Persistence Manager (C)
add_library(F05_StatePersistenceManager
    src/F05_StatePersistenceManager.c
)
target_include_directories(F05_StatePersistenceManager
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-06: HMI and Event Logger (C)
add_library(F06_HmiAndEventLogger
    src/F06_HmiAndEventLogger.c
)
target_include_directories(F06_HmiAndEventLogger
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-07: Rear Risk Evaluation (C)
add_library(F07_RearRiskEvaluation
    src/F07_RearRiskEvaluation.c
)
target_include_directories(F07_RearRiskEvaluation
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-08: Rear Risk Protection Controller (C)
add_library(F08_RearRiskProtectionController
    src/F08_RearRiskProtectionController.c
)
target_include_directories(F08_RearRiskProtectionController
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-09: Rear Seat Occupancy Alert (C)
add_library(F09_RearSeatOccupancyAlert
    src/F09_RearSeatOccupancyAlert.c
)
target_include_directories(F09_RearSeatOccupancyAlert
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-10: Ignition Off Status Alert (C)
add_library(F10_IgnitionOffStatusAlert
    src/F10_IgnitionOffStatusAlert.c
)
target_include_directories(F10_IgnitionOffStatusAlert
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

enable_testing()
add_subdirectory(test)
