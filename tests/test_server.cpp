#include <gtest/gtest.h>

TEST(SerialServerTest, ConnectionTest) {
    EXPECT_EQ(1, 1);  // Dummy test for illustration
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
