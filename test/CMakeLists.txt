# Testing uses CTest
enable_testing()

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Gather source files
set(TEST_SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/F08_RearRiskProtectionController.c
)

# Register tests
include(GoogleTest)

# F-08 Unit Test Target
add_executable(test_F08_RearRiskProtection
    test_F08_RearRiskProtectionController.cpp
    ${TEST_SRC_FILES}
)
target_link_libraries(test_F08_RearRiskProtection gtest_main)
gtest_discover_tests(test_F08_RearRiskProtection)
# Gather source files (excluding main, since we'll test individual units)
# For now, we only have F07, but we can make it extensible
set(TEST_SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/F07_RearRiskEvaluation.c
)

# F-07 Unit Test Target
add_executable(test_F07_RearRiskEvaluation
    test_F07_RearRiskEvaluation.cpp
    ${TEST_SRC_FILES}
)

# Link against Google Test
target_link_libraries(test_F07_RearRiskEvaluation
    gtest_main
)

# Register tests
include(GoogleTest)
gtest_discover_tests(test_F07_RearRiskEvaluation)
