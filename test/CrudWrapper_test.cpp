#include "crud-wrapper/Common.hpp"

#include "crud-wrapper/CrudWrapper.hpp"

#include "gtest/gtest.h"

/// @brief namespace for arrayAdt_test tests
namespace sql_with_cpp_test::crudWrapper_test {
using namespace ::sql_with_cpp;

TEST(TestingConstruction, ConstructingCrudWrapper) {
  EXPECT_THROW(
      { CrudWrapper{"/non/existing/path"}; },
      std::filesystem::filesystem_error);

  EXPECT_NO_THROW({ CrudWrapper{common::kprojectRootPath + "/db/album.db"}; });
  EXPECT_NO_THROW(
      { CrudWrapper{common::kprojectRootPath + "/db/scratch.db"}; });
  EXPECT_NO_THROW({ CrudWrapper{common::kprojectRootPath + "/db/world.db"}; });
}

} // namespace sql_with_cpp_test::crudWrapper_test
