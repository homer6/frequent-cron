#include <gtest/gtest.h>

#include <hello2.h>

TEST(hello2, test1)
{
  ASSERT_EQ(3, hello2(1, 2));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
