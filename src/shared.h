#pragma once

//#include <condition_variable>
//#include <thread>
//#include <atomic>
//#include <opencv2/core.hpp>
//
//#include "httplib.h"

#include "stats.h"

struct Options
{
    bool        showDebugWindows;
    int         cameraIndex;
    int         httpPort;
    std::string filename;
    int         video_playback_slowdown_factor;

    Options() :
          showDebugWindows(false)
        , cameraIndex(0)
        , httpPort(9080)
        , video_playback_slowdown_factor(1)
    {}
};

/*
        $scope.data.rowCount         = $scope.rowCountSlider.value;
        $scope.data.rowSpacePx       = $scope.rowSpaceSlider.value;
        $scope.data.rowPerspectivePx = $scope.rowPerspectiveSlider.value;
*/

struct DetectSettings {
    public:
        int         frame_cols;
        int         frame_rows;
        cv::Scalar  colorFrom;
        cv::Scalar  colorTo;
        int         maxPlats;
        int         maxZentimeter;
        int         erode_iterations;
        int         dilate_iterations;
        int         minimalContourArea;
        int         rowCount;
        int         rowSpacePx;
        int         rowPerspectivePx;

    DetectSettings()
    {
        frame_cols         = 640;
        frame_rows         = 480;
        colorFrom          = cv::Scalar(36,  25,  25);
        colorTo            = cv::Scalar(86, 255, 255);
        erode_iterations   = 5;
        dilate_iterations  = 5;
        maxPlats           = 10;
        maxZentimeter      = 5;
        minimalContourArea = 130;
        rowCount           = 1;
        rowSpacePx         = 160;
        rowPerspectivePx   = 0;

    }
};

struct Shared {
    public:

        Stats                   stats;
        httplib::Server         *webSvr;

        std::atomic<bool>       shutdown_requested;
        //
        // sync between camera an detection thread
        //
        cv::Mat                 frame_buf[3];
        std::atomic<int>        frame_buf_slot;
        std::condition_variable camera_frame_ready;
        std::mutex              camera_frame_ready_mutex;
        //
        // sync between detection and sending thread
        //
        cv::Mat                 analyzed_frame_buf[2];
        std::mutex              analyzed_frame_buf_mutex[2];
        std::condition_variable analyzed_frame_ready;
        std::mutex              analyzed_frame_ready_mutex;
        std::atomic<int>        analyzed_frame_ready_idx;
        std::atomic<bool>       analyzed_frame_encoded_to_JPEG;
        std::vector<uchar>      analyzed_frame_jpegImage;
        //
        // 
        //
        DetectSettings         detectSettings;
    Shared()
    {
        shutdown_requested.store(false);
        webSvr = nullptr;
        frame_buf_slot.store(-1);
        analyzed_frame_ready_idx.store(-1);
        analyzed_frame_encoded_to_JPEG.store(false);
    }
};