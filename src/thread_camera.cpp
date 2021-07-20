#include <atomic>
#include <opencv2/opencv.hpp>

#include "stats.h"

void thread_camera(Stats* stats, cv::Mat frame_buf[], std::atomic<int>* sharedFrameBufSlot)
{
    cv::VideoCapture capture = cv::VideoCapture(0);

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

    *sharedFrameBufSlot = 1;
    WorkNr = 1;
    CurrNr = 2;

    for (;;)
    {
        if ( !capture.read(frame_buf[CurrNr - 1]) )
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

            stats->camera_frames_read++;
        }
    }
}