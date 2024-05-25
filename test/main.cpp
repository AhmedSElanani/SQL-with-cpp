#include "gtest/gtest.h"

/// @brief main entry point for tests
/// @param argc
/// @param argv
/// @return status from executing the test cases
auto main(int argc, char *argv[]) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
