# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# F-01 Unit Test
add_executable(test_F01_InputMonitorAndValidator
    test_F01_InputMonitorAndValidator.cpp
)
target_link_libraries(test_F01_InputMonitorAndValidator
    PRIVATE F01_InputMonitorAndValidator gtest_main
)
gtest_discover_tests(test_F01_InputMonitorAndValidator)

# F-02 Unit Test (GoogleTest)
add_executable(test_F02_ChildLockStateDecision
    test_F02_ChildLockStateDecision.cpp
)
target_link_libraries(test_F02_ChildLockStateDecision
    PRIVATE F02_ChildLockStateDecision gtest_main
)
gtest_discover_tests(test_F02_ChildLockStateDecision)

# F-03 Unit Test
add_executable(test_F03_DoorEcuCommandHandler
    test_F03_DoorEcuCommandHandler.cpp
)
target_link_libraries(test_F03_DoorEcuCommandHandler
    PRIVATE F03_DoorEcuCommandHandler gtest_main
)
gtest_discover_tests(test_F03_DoorEcuCommandHandler)

# F-04 Unit Test
add_executable(test_F04_RearDoorOpenBlockHandler
    test_F04_RearDoorOpenBlockHandler.cpp
)
target_link_libraries(test_F04_RearDoorOpenBlockHandler
    PRIVATE F04_RearDoorOpenBlockHandler gtest_main
)
gtest_discover_tests(test_F04_RearDoorOpenBlockHandler)

# F-05 Unit Test
add_executable(test_F05_StatePersistenceManager
    test_F05_StatePersistenceManager.cpp
)
target_link_libraries(test_F05_StatePersistenceManager
    PRIVATE F05_StatePersistenceManager gtest_main
)
gtest_discover_tests(test_F05_StatePersistenceManager)

# F-06 Unit Test
add_executable(test_F06_HmiAndEventLogger
    test_F06_HmiAndEventLogger.cpp
)
target_include_directories(test_F06_HmiAndEventLogger PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_F06_HmiAndEventLogger
    PRIVATE F06_HmiAndEventLogger gtest_main
)
gtest_discover_tests(test_F06_HmiAndEventLogger)

# F-07 Unit Test
add_executable(test_F07_RearRiskEvaluation
    test_F07_RearRiskEvaluation.cpp
)
target_link_libraries(test_F07_RearRiskEvaluation
    PRIVATE F07_RearRiskEvaluation gtest_main
)
gtest_discover_tests(test_F07_RearRiskEvaluation)

# F-08 Unit Test
add_executable(test_F08_RearRiskProtection
    test_F08_RearRiskProtectionController.cpp
)
target_link_libraries(test_F08_RearRiskProtection
    PRIVATE F08_RearRiskProtectionController gtest_main
)
gtest_discover_tests(test_F08_RearRiskProtection)

# F-09 Unit Test
add_executable(test_F09_RearSeatOccupancyAlert
    test_F09_RearSeatOccupancyAlert.cpp
)
target_include_directories(test_F09_RearSeatOccupancyAlert PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_F09_RearSeatOccupancyAlert
    PRIVATE F09_RearSeatOccupancyAlert gtest_main
)
gtest_discover_tests(test_F09_RearSeatOccupancyAlert)

# F-10 Unit Test
add_executable(test_F10_IgnitionOffStatusAlert
    test_F10_IgnitionOffStatusAlert.cpp
)
target_link_libraries(test_F10_IgnitionOffStatusAlert
    PRIVATE F10_IgnitionOffStatusAlert gtest_main
)
gtest_discover_tests(test_F10_IgnitionOffStatusAlert)
