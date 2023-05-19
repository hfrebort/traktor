#pragma once

#include "shared.h"

class CameraContext
{
    public:
    
    Stats*              stats;
    const Options*      options;
    Shared*             shared;
    cv::VideoCapture    capture;

    int                 errorCount = 0;
    int delay_for_realtime_video_millis = 0;

    CameraContext(Stats* stats, const Options* options, Shared* shared)
    : stats(stats)
    , options(options)
    , shared(shared)
    {}
};

