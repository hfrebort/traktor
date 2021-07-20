#pragma once

#include <cstddef>
#include <atomic>

struct Stats
{
    public:
        std::atomic<uint64_t>                   fps;
        std::atomic<uint64_t>                   camera_frames;
        std::atomic<uint64_t>                   camera_read_ns;
        std::atomic<uint64_t>                   jpeg_sent;

    Stats() :
        fps(0)
        , camera_frames(0)
        , jpeg_sent(0) 
        , camera_read_ns(0) {}
};