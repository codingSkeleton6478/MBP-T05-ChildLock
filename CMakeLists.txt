cmake_minimum_required(VERSION 3.22)

project(child_lock_state_decision LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(child_lock_state_decision
    src/child_lock_state_decision.cpp
)

target_include_directories(child_lock_state_decision
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(child_lock_state_decision_test
    test/child_lock_state_decision_test.cpp
)

target_link_libraries(child_lock_state_decision_test
    PRIVATE
        child_lock_state_decision
)

enable_testing()

add_test(
    NAME child_lock_state_decision_test
    COMMAND child_lock_state_decision_test
)
project(MBP-T05-ChildLock VERSION 1.0.0 LANGUAGES C CXX)

# Default to C11 and C++17
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compile options for strict checking (MISRA friendly)
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wconversion -Wsign-conversion
    -Wformat=2 -Wundef -Wshadow
    -Wcast-align -Wstrict-prototypes -Wmissing-prototypes
    -Wmissing-declarations -Wredundant-decls
)

# Coverage support
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    message(STATUS "Coverage enabled")
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

# Directories
include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(test)
