#pragma once

#include "shared.h"
#include "stats.h"

struct EncodeContext
{
    Shared* shared;
    Stats* stats;
    std::unique_ptr<cv::Mat> status_bar = nullptr;
    std::vector<uchar> jpeg_buffer;
    std::function<bool(std::vector<uchar>&)> sendJPEGbytes;

    EncodeContext(Stats* stats, Shared* shared, std::function<bool(std::vector<uchar>&)> sendJPEGbytes)
    : stats(stats)
    , shared(shared)
    , sendJPEGbytes(sendJPEGbytes)
    {}
};

