#include "shared.h"

struct CameraContext
{
    Stats*              stats;
    const Options*      options;
    Shared*             shared;

    cv::VideoCapture    capture;

    int                 errorCount = 0;
    int delay_for_realtime_video_millis = 0;
};