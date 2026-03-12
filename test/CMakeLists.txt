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

# F-05 Unit Test
add_executable(test_state_persistence_manager
    test_state_persistence_manager.cpp
)
target_link_libraries(test_state_persistence_manager
    PRIVATE StatePersistenceManager gtest_main
)
gtest_discover_tests(test_state_persistence_manager)

# Child Lock State Decision Unit Test (custom runner, no GoogleTest)
add_executable(child_lock_state_decision_test
    child_lock_state_decision_test.cpp
)
target_link_libraries(child_lock_state_decision_test
    PRIVATE child_lock_state_decision
)
add_test(NAME child_lock_state_decision_test COMMAND child_lock_state_decision_test)
