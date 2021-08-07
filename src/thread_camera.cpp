#include <atomic>
#include <opencv2/opencv.hpp>

#include "shared.h"
#include "stats.h"

void thread_camera(int cameraIdx, Shared* shared)
{
    cv::VideoCapture capture = cv::VideoCapture(cameraIdx);
    
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    //capture.set(cv::CAP_PROP_BUFFERSIZE, 1);

    printf("camera: %dx%d fps: %d\n",
         (int)capture.get(cv::CAP_PROP_FRAME_WIDTH)
        ,(int)capture.get(cv::CAP_PROP_FRAME_HEIGHT)
        ,(int)capture.get(cv::CAP_PROP_FPS));

    short WorkNr;
    short CurrNr = 0;
    short NextNr; 
    short PrevNr;

    if ( !capture.read(shared->frame_buf[CurrNr]) )
    {
        printf("E: first capture.read()\n");
        return;
    }

    printf("I: thread camera running\n");

    shared->frame_buf_slot.store(0);
    WorkNr = 0;
    CurrNr = 1;

    for (;;)
    {
        cv::Mat& buf = shared->frame_buf[CurrNr];

        bool read_ok = capture.read(buf);
        shared->stats.camera_frames++;

        if ( !read_ok )
        {
            printf("E: capture.read()\n");
            break;
        }
        else
        {
            PrevNr = std::atomic_exchange( &(shared->frame_buf_slot), CurrNr );
            shared->camera_frame_ready.notify_one();
            NextNr = ( PrevNr == -1 ) ? 3 - ( WorkNr + CurrNr ) : PrevNr;
            WorkNr = CurrNr;
            CurrNr = NextNr;
        }

        if (shared->shutdown_requested.load())
        {
            printf("I: thread camera shutdown requested\n");
            break;
        }
    }
}