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

# F-05: State Persistence Manager (C)
add_library(StatePersistenceManager
    src/childlock/state_persistence_manager.c
)
target_include_directories(StatePersistenceManager
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-10: Ignition Off Status Alert (C)
add_library(IgnitionOffStatusAlert
    src/childlock/ignition_off_status_alert.c
)
target_include_directories(IgnitionOffStatusAlert
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-03: Door ECU Command Handler (C)
add_library(DoorEcuCommandHandler
    src/childlock/door_ecu_command_handler.c
)
target_include_directories(DoorEcuCommandHandler
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# F-04: Rear Door Open Block Handler (C)
add_library(RearDoorOpenBlockHandler
    src/childlock/rear_door_open_block_handler.c
)
target_include_directories(RearDoorOpenBlockHandler
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# Child Lock State Decision (C++)
add_library(child_lock_state_decision
    src/child_lock_state_decision.cpp
)
target_include_directories(child_lock_state_decision
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

enable_testing()
add_subdirectory(test)
