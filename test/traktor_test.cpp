#include "gtest/gtest.h"

#include "../src/shared.h"

bool find_point_on_nearest_refline(
      const cv::Point&        plant
    , const ReflinesSettings& settings
    , float  *nearest_refLine_x
    , float  *deltaPx
    , float  *refLines_distance_px);

TEST(TraktorTests, Test_plant_on_middle_line)
{
    auto plant = cv::Point(0,0);
    ReflinesSettings refline;
    refline.y_fluchtpunkt = 0;

    ASSERT_EQ(0, plant.x + plant.y);
}

TEST(TraktorTests, TestIntegerOne_One)
{
    const auto expected = 1;
    const auto actual = 1 * 2;
    ASSERT_EQ(expected, actual);
}

