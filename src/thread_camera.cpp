#include <atomic>
#include <opencv2/opencv.hpp>

#include "stats.h"

void thread_camera(int cameraIdx, Stats* stats, cv::Mat frame_buf[3], std::atomic<int>* sharedFrameBufSlot)
{
    cv::VideoCapture capture = cv::VideoCapture(cameraIdx);
    
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    //capture.set(cv::CAP_PROP_BUFFERSIZE, 1);

    printf("camera: %dx%d fps: %d\n",
         (int)capture.get(cv::CAP_PROP_FRAME_WIDTH)
        ,(int)capture.get(cv::CAP_PROP_FRAME_HEIGHT)
        ,(int)capture.get(cv::CAP_PROP_FPS));

    int WorkNr;
    int CurrNr = 1;
    int NextNr; 
    int PrevNr;

    if ( !capture.read(frame_buf[CurrNr - 1]) )
    {
        printf("E: first capture.read()\n");
        return;
    }

    printf("I: thread camera running\n");

    sharedFrameBufSlot->store(1);
    WorkNr = 1;
    CurrNr = 2;

    for (;;)
    {
        cv::Mat& buf = frame_buf[CurrNr - 1];

        bool read_ok = capture.read(buf);
        stats->camera_frames++;

        if ( !read_ok )
        {
            printf("E: capture.read()\n");
            break;
        }
        else
        {
            PrevNr = std::atomic_exchange( sharedFrameBufSlot, CurrNr );
            NextNr = ( PrevNr == 0 ) ? 6 - ( WorkNr + CurrNr ) : PrevNr;
            WorkNr = CurrNr;
            CurrNr = NextNr;
        }
    }
}