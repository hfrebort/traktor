#pragma once

#include <opencv2/videoio.hpp>

#include "shared.h"

class CameraContext
{
    public:
    
    CameraCounter*      stats;
    const Options*      options;
    Shared*             shared;
    std::unique_ptr<cv::VideoCapture> capture;

    int                 errorCount = 0;
    int delay_for_realtime_video_millis;

    CameraContext(CameraCounter* stats, const Options* options, Shared* shared)
    : stats(stats)
    , options(options)
    , shared(shared)
    , delay_for_realtime_video_millis(0)
    , errorCount(0)
    , capture(nullptr)
    {}
};

