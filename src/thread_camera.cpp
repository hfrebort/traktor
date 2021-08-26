//#include <atomic>
//#include <opencv2/opencv.hpp>

#include "shared.h"
#include "stats.h"

void thread_camera(int cameraIdx, Shared* shared)
{
    cv::VideoCapture capture = cv::VideoCapture(cameraIdx);
    
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    //capture.set(cv::CAP_PROP_BUFFERSIZE, 1);

    int CurrNr = 0;

    if ( !capture.read(shared->frame_buf[CurrNr]) )
    {
        printf("E: first capture.read()\n");
        return;
    }

    printf("I: thread camera: running. framesize: %dx%d, CAP_PROP_FPS: %d\n",
         shared->frame_buf[CurrNr].cols
        ,shared->frame_buf[CurrNr].rows
        ,(int)capture.get(cv::CAP_PROP_FPS));

    shared->frame_buf_slot.store(CurrNr);
    
    int WorkNr = CurrNr;
        CurrNr = 1;

    for (;;)
    {
        if ( ! capture.read( shared->frame_buf[CurrNr] ) )
        {
            printf("E: capture.read()\n");
            break;
        }
        else
        {
            int PrevNr = std::atomic_exchange( &(shared->frame_buf_slot), CurrNr );
            int NextNr = ( PrevNr == -1 ) ? 3 - ( WorkNr + CurrNr ) : PrevNr;
                WorkNr = CurrNr;
                CurrNr = NextNr;

            shared->camera_frame_ready.notify_one();
            shared->stats.camera_frames++;
        }

        if (shared->shutdown_requested.load())
        {
            printf("I: thread camera: shutdown requested\n");
            break;
        }
    }
    printf("I: thread camera: bye...\n");
}