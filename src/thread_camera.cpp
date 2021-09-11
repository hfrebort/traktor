//#include <atomic>
//#include <opencv2/opencv.hpp>

#include "shared.h"
#include "stats.h"

void thread_camera(const Options& options, Shared* shared)
{
    cv::VideoCapture capture = options.filename.empty() ? cv::VideoCapture(options.cameraIndex) : cv::VideoCapture(options.filename);
    
    capture.set(cv::CAP_PROP_FRAME_WIDTH,  640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture.set(cv::CAP_PROP_FPS,           30);
    //capture.set(cv::CAP_PROP_BUFFERSIZE, 1);

    int CurrNr = 0;

    if ( !capture.read(shared->frame_buf[CurrNr]) )
    {
        printf("E: first capture.read()\n");
        return;
    }

    const int cap_prop_fps = (int)capture.get(cv::CAP_PROP_FPS);
    const int delay_for_realtime_video_millis = options.filename.empty() ? 0 : 1000 / cap_prop_fps * options.video_playback_slowdown_factor;

    DetectSettings& decset = shared->detectSettings;

    decset.frame_cols = shared->frame_buf[CurrNr].cols;
    decset.frame_rows = shared->frame_buf[CurrNr].rows;

    printf("I: thread camera: running. framesize: %dx%d, CAP_PROP_FPS: %d\n",
         shared->frame_buf[CurrNr].cols
        ,shared->frame_buf[CurrNr].rows
        ,cap_prop_fps);

    shared->frame_buf_slot.store(CurrNr);
    
    int WorkNr = CurrNr;
        CurrNr = 1;

    if ( options.cropPx > 0 )
    {
        shared->detectSettings.frame_cols -= 2 * options.cropPx;
        printf("set frame_cols to: %d\n", decset.frame_cols);
    }

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
            if (options.cropPx > 0)
            {   
                const int x_start =  options.cropPx + decset.hydroPx;
                cv::Rect region( x_start, 0, decset.frame_cols, decset.frame_rows);
                shared->frame_buf[CurrNr] = shared->frame_buf[CurrNr](region);   
            }

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