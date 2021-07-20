#pragma once

#include <cstddef>
#include <atomic>

struct Stats
{
    public:
        std::atomic<uint64_t> fps;
        std::atomic<uint64_t> camera_frames_read;
        std::atomic<uint64_t> jpeg_sent;

};