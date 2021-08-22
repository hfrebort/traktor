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

}