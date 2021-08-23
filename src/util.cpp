#include "util.h"

namespace trk
{

int64_t getDuration_ns(std::chrono::_V2::system_clock::time_point *start)
{
    auto finish = std::chrono::high_resolution_clock::now();
    int64 duration = std::chrono::duration_cast<std::chrono::nanoseconds>( finish - *start).count();

    *start = finish;

    return duration;
}

/*
data: {
  "colorFilter": true,
  "colorFrom": "36,80,25",
  "colorTo": "149,160,255",
*/
bool setColorFromCSV(const std::string& csv, cv::Scalar& color)
{
    int h,s,v;
    int successfull_items_scaned = sscanf(csv.c_str(), "%d,%d,%d", &h, &s, &v );
    color[0] = h;
    color[1] = s;
    color[2] = v;
    return successfull_items_scaned == 3;
}

} // namespace