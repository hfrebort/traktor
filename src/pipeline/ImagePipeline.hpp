#pragma once

#include <atomic>
#include <functional>
#include <thread>

#include <opencv2/core.hpp>

#include "auto_reset_event.hpp"
#include "Postbox.hpp"

#include "../camera.h"
#include "../detect.h"
#include "../encode.h"

enum WORKER_RC
{
    OK,
    LIKE_TO_EXIT
};

struct Workitem {
    cv::Mat frame;
    std::atomic<bool>   isPictureFromCamera;
    DetectResult        detect_result;
};

class ImagePipeline
{
public:

    ImagePipeline();

    void start_camera_1(std::function<void(Workitem*,CameraContext*)> process, CameraContext* context);
    void start_detect_2(std::function<void(Workitem*,DetectContext*)> process, DetectContext* context);
    void run_encode_3(std::function<WORKER_RC(Workitem*,EncodeContext*)> process, EncodeContext* context);
    void shutdown();

private:

    #define NO_FREE_SLOT_FOUND (-1)

    #define ACTION_FREE  -1
    #define ACTION_SLEEP -2
    #define ACTION_EXIT  -3

    #define ACTION_MIN   -3
    #define ACTION_MAX    4

    Postbox _CameraToDetect{ ACTION_FREE };
    Postbox _DetectToEncode{ ACTION_FREE };

    #define NUMBER_SLOTS 5
    std::atomic_bool    _Free      [ NUMBER_SLOTS ];
    Workitem            _workitems [ NUMBER_SLOTS ];

    std::thread         _threads[ 3 ];

    bool read (Postbox& postbox, int32_t* idx);
    bool write(Postbox& postbox, int32_t write_idx);

    int8_t get_free();
    void   set_free(int8_t idx);
    void   set_all_free();
    void   set_all_full();

    void camera_1(std::function<void     (Workitem*,CameraContext*)> process, CameraContext* context);
    void detect_2(std::function<void     (Workitem*,DetectContext*)> process, DetectContext* context);
    void encode_3(std::function<WORKER_RC(Workitem*,EncodeContext*)> process, EncodeContext* context);

};