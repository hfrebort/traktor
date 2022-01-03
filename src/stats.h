#pragma once

#include <cstdint>
#include <atomic>
#include <condition_variable>

struct Stats
{
    public:
        std::atomic<int>        fps;
        std::atomic<int>        camera_frames;
        std::atomic<uint64_t>   camera_read_ns;
        std::atomic<uint64_t>   jpeg_sent;
        std::atomic<uint64_t>   jpeg_bytes_sent;

        std::atomic<uint64_t>   detect_overall_ns;
        std::atomic<uint64_t>   prepare_ns;
        std::atomic<uint64_t>   findContours_ns;
        std::atomic<uint64_t>   calc_draw_ns;
        
        std::atomic<uint64_t>   frame_bytes_processed;

        std::atomic<uint64_t>   prepare_cvtColor_ns;
        std::atomic<uint64_t>   prepare_GaussianBlur_ns;
        std::atomic<uint64_t>   prepare_inRange_ns;
        std::atomic<uint64_t>   prepare_erode_ns;
        std::atomic<uint64_t>   prepare_dilate_ns;


    Stats() :
        fps(0)
        , camera_frames(0)
        , jpeg_sent(0) 
        , camera_read_ns(0) 
        , prepare_ns(0)
        , findContours_ns(0)
        , detect_overall_ns(0)
        , jpeg_bytes_sent(0)
        , frame_bytes_processed(0)
        , prepare_cvtColor_ns(0)
        , prepare_GaussianBlur_ns(0)
        , prepare_inRange_ns(0)
        , prepare_erode_ns(0)
        , prepare_dilate_ns(0)
        {}
};