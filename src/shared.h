#pragma once
#include <atomic>
#include <opencv2/opencv.hpp>

#include "stats.h"

struct Shared {
    public:

        Stats               stats;

        cv::Mat             frame_buf[3];
        std::atomic<int>    frame_buf_slot;

};