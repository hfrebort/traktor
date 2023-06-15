#include <gtest/gtest.h>

TEST(TraktorTests, TestIntegerOne_One2)
{
    const auto expected = 1;
    const auto actual = 1 * 1;
    ASSERT_EQ(expected, actual);
}

TEST(TraktorTests, TestIntegerOne_One)
{
    const auto expected = 1;
    const auto actual = 1 * 2;
    ASSERT_EQ(expected, actual);
}

