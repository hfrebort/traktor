#pragma once

#include <condition_variable>
#include <thread>
#include <atomic>
#include <opencv2/opencv.hpp>

#include "httplib.h"

#include "stats.h"

struct Shared {
    public:

        Stats                   stats;
        httplib::Server         *webSvr;

        std::atomic<bool>       shutdown_requested;
        //
        // sync between camera an detection thread
        //
        cv::Mat                 frame_buf[3];
        std::atomic<short>      frame_buf_slot;
        std::condition_variable camera_frame_ready;
        std::mutex              camera_frame_ready_mutex;
        //
        // sync between detection and sending thread
        //
        cv::Mat                 analyzed_frame_buf[2];
        std::mutex              analyzed_frame_buf_mutex[2];
        std::condition_variable analyzed_frame_ready;
        std::mutex              analyzed_frame_ready_mutex;
        std::atomic<short>      analyzed_frame_ready_idx;

    Shared()
    {
        shutdown_requested.store(false);
        webSvr = nullptr;
    }
};