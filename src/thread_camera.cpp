//#include <atomic>
//#include <opencv2/opencv.hpp>

#include "shared.h"
#include "stats.h"

void thread_camera(const Options& options, Shared* shared)
{
    cv::VideoCapture capture = options.filename.empty() ? cv::VideoCapture(options.cameraIndex) : cv::VideoCapture(options.filename);
    
    if ( options.filename.empty() ) 
    {
        
        capture.set(cv::CAP_PROP_FRAME_WIDTH,  (double)options.camera_width);
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, (double)options.camera_height);
        printf("I: set CAP_PROP_FRAME_WIDTH x CAP_PROP_FRAME_HEIGHT = %dx%d\n", options.camera_width, options.camera_height);

        //capture.set(cv::CAP_PROP_FPS,           (double)options.camera_fps);
        //capture.set(cv::CAP_PROP_BUFFERSIZE, 1);
    }

    int CurrNr = 0;

    if ( !capture.read(shared->frame_buf[CurrNr]) )
    {
        printf("E: first capture.read()\n");
        return;
    }

    const int cap_prop_fps                    = (int)capture.get(cv::CAP_PROP_FPS);
    const int delay_for_realtime_video_millis = options.filename.empty() ? 0 : 1000 / cap_prop_fps * options.video_playback_slowdown_factor;

    DetectSettings& settings = shared->detectSettings;
    settings.set_frame( shared->frame_buf[CurrNr].cols
                      , shared->frame_buf[CurrNr].rows );

    printf("I: thread camera: running. framesize: %dx%d, CAP_PROP_FPS: %d\n",
         shared->frame_buf[CurrNr].cols
        ,shared->frame_buf[CurrNr].rows
        ,cap_prop_fps);

    shared->frame_buf_slot.store(CurrNr);
    
    int WorkNr = CurrNr;
        CurrNr = 1;

    for (;;)
    {
        if ( delay_for_realtime_video_millis != 0)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(delay_for_realtime_video_millis) );
        }

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
            break;
        }
    }
}