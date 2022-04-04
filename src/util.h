#pragma once
//#include <chrono>

namespace trk
{

int64_t getDuration_ns  (std::chrono::_V2::system_clock::time_point *start);
bool    setColorFromCSV (const std::string& csv, cv::Scalar& color);
void    set_color_from_values(int h, int s, int v, cv::Scalar& color);

}