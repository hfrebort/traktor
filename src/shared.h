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


        cv::Mat                 frame_buf[3];
        std::atomic<short>      frame_buf_slot;

        std::condition_variable camera_frame_ready;
        std::mutex              camera_frame_ready_mutex;

    Shared()
    {
        shutdown_requested.store(false);
        webSvr = nullptr;
    }
};