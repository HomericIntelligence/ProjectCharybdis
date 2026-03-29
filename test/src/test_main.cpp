#include "projectcharybdis/version.hpp"

#include <gtest/gtest.h>

namespace projectcharybdis::test {

TEST(VersionTest, ProjectNameIsCorrect) {
  EXPECT_EQ(kProjectName, "ProjectCharybdis");
}

TEST(VersionTest, VersionIsSet) {
  EXPECT_FALSE(kVersion.empty());
}

} // namespace projectcharybdis::test
