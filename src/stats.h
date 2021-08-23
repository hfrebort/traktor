#pragma once

//#include <cstddef>
//#include <atomic>

struct Stats
{
    public:
        std::atomic<int>        fps;
        std::atomic<uint64_t>   camera_frames;
        std::atomic<uint64_t>   camera_read_ns;
        std::atomic<uint64_t>   jpeg_sent;
        std::atomic<uint64_t>   jpeg_bytes_sent;
        std::atomic<uint64_t>   prepare_ns;
        std::atomic<uint64_t>   detect_overall_ns;
        std::atomic<uint64_t>   findContours_ns;
        std::atomic<uint64_t>   calcCenters_ns;
        std::atomic<uint64_t>   draw_ns;

    Stats() :
        fps(0)
        , camera_frames(0)
        , jpeg_sent(0) 
        , camera_read_ns(0) 
        , prepare_ns(0)
        , findContours_ns(0)
        , detect_overall_ns(0)
        , jpeg_bytes_sent(0)
        {}
};