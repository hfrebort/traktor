//#include <atomic>
//#include <opencv2/opencv.hpp>

#include "shared.h"
#include "stats.h"
#include "camera.h"

#include "pipeline/ImagePipeline.hpp"

const cv::Scalar RED   = cv::Scalar(0,0,255);
const cv::Scalar BLUE  = cv::Scalar(255,0,0);
const cv::Scalar GREEN = cv::Scalar(0,255,0);

void copy_status_frame_to(cv::OutputArray dest, const char* text, const cv::Scalar& color, const int retryCount) {

    cv::String msg(text);

    if (retryCount >= 0) {
        msg += " #" + std::to_string(retryCount);
    }

    cv::Mat status( cv::Mat::zeros(480, 640, 16) );
    cv::putText(status, msg, cv::Point(10, 240), cv::FONT_HERSHEY_SIMPLEX, 1, color, 2);
    status.copyTo(dest);
}

bool try_open_capture(cv::VideoCapture* capture, const Options* options)
{
    if ( options->filename.empty() ) {
        printf("I: opening camera #%d...\n", options->cameraIndex);
        if ( capture->open(options->cameraIndex) ) {
            capture->set(cv::CAP_PROP_FRAME_WIDTH,  (double)options->camera_width);
            capture->set(cv::CAP_PROP_FRAME_HEIGHT, (double)options->camera_height);
            printf("I: set CAP_PROP_FRAME_WIDTH x CAP_PROP_FRAME_HEIGHT = %dx%d\n", options->camera_width, options->camera_height);
        }
    }
    else {
        printf("I: opening file %s\n", options->filename.c_str());
        capture->open(options->filename);
    }

    return capture->isOpened();
}

bool ensure_camera_open( Workitem* work, CameraContext* ctx )
{
    if ( ! ctx->capture->isOpened() ) 
    {
        puts("going to open capture...");

        if ( ctx->errorCount > 0 ) {
            std::this_thread::sleep_for( std::chrono::seconds(1) );
        }

        if ( ! try_open_capture(ctx->capture.get(), ctx->options) ) {
            fprintf(stderr,"E: cannot open capture device. retry...\n");
            copy_status_frame_to(work->frame, "could not open camera", RED, ctx->errorCount);
            ctx->errorCount += 1;
            return false;
        }
        else
        {
            puts("I: capture device opened successfully");
            return true;
        }
    }
    else
    {
        return true;
    }
}

void camera_main(Workitem* work, CameraContext* ctx)
{
    if ( ctx->capture == nullptr )
    {
        ctx->errorCount = 1;    // hacky. set error here to get set_frame() called. Do it better. TODO

        // call the constructor here in the camera thread
        puts("VideoCapure ctor...");
        ctx->capture = std::make_unique<cv::VideoCapture>();
        puts("VideoCapure ctor...done");
    }
    //cv::VideoCapture& capture = ctx->capture.get();
    const Options* options = ctx->options;

    /*
    if ( ctx->delay_for_realtime_video_millis != 0)
    {
        std::this_thread::sleep_for( std::chrono::milliseconds(ctx->delay_for_realtime_video_millis) );
    }*/

    if ( ensure_camera_open(work,ctx) ) {
        if ( ! ctx->capture->read( work->frame ) )
        {
            ctx->capture->release();
            ctx->errorCount += 1;
            fprintf(stderr, "E: capture.read()\n");
            copy_status_frame_to(work->frame, "could not read from camera", RED, ctx->errorCount);
        }
        else 
        {   
            ctx->stats->frames++;

            if ( ctx->errorCount > 0 ) 
            {
                ctx->errorCount = 0;
                ctx->delay_for_realtime_video_millis = options->filename.empty() 
                                                                ? 0 
                                                                : 1000 / ((int)ctx->capture->get(cv::CAP_PROP_FPS)) * options->video_playback_slowdown_factor;
                ctx->shared->detectSettings.set_frame( work->frame.cols
                                                    , work->frame.rows );
                /*
                printf("I: COLS and ROWS were set to: %dx%d\n", 
                     ctx->shared->detectSettings.getImageSettings().frame_cols
                    ,ctx->shared->detectSettings.getImageSettings().frame_rows);
                */
            }
        }
    }
    work->isValidForAnalyse = ( ctx->errorCount == 0 );
}
/*
void thread_camera(const Options& options, Shared* shared)
{
    cv::VideoCapture capture;
    
    int CurrNr = 0;

    copy_status_frame_to(shared->frame_buf[CurrNr], "starting up...", GREEN, -1);
    shared->detectSettings.detecting.store(false);
    
    shared->frame_buf_slot.store(CurrNr);
    int WorkNr = CurrNr;
        CurrNr = 1;

    int delay_for_realtime_video_millis = 0;
    bool should_set_cols_and_rows = true;
    int retryCount = 0;

    for (;;)
    {
        if ( delay_for_realtime_video_millis != 0)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(delay_for_realtime_video_millis) );
        }

        if ( ! capture.isOpened() ) 
        {
            if ( ! try_open_capture(&capture, options, shared) ) {
                fprintf(stderr,"E: cannot open capture device. retry...\n");
                copy_status_frame_to(shared->frame_buf[CurrNr], "could not open camera", RED, retryCount);
                retryCount += 1;
            }
            else
            {
                printf("I: capture device opened successfully\n");
                copy_status_frame_to(shared->frame_buf[CurrNr], "camera opened", GREEN, -1);
            }
            shared->detectSettings.detecting.store(false);
            should_set_cols_and_rows = true;
        }
        else if ( ! capture.read( shared->frame_buf[CurrNr] ) )
        {
            fprintf(stderr, "E: capture.read()\n");
            capture.release();
            copy_status_frame_to(shared->frame_buf[CurrNr], "could not read from camera", RED, retryCount);
            should_set_cols_and_rows = true;
            shared->detectSettings.detecting.store(false);
            retryCount += 1;
        }
        else if ( should_set_cols_and_rows ) {
            should_set_cols_and_rows = false;
            retryCount = 0;

            delay_for_realtime_video_millis = options.filename.empty() ? 0 : 1000 / ((int)capture.get(cv::CAP_PROP_FPS)) * options.video_playback_slowdown_factor;
            shared->detectSettings.set_frame( shared->frame_buf[CurrNr].cols
                                            , shared->frame_buf[CurrNr].rows );
            shared->detectSettings.detecting.store(true);
            printf("I: COLS and ROWS were set to: %dx%d\n", shared->frame_buf[CurrNr].cols, shared->frame_buf[CurrNr].rows);
        }

        int PrevNr = std::atomic_exchange( &(shared->frame_buf_slot), CurrNr );
        int NextNr = ( PrevNr == -1 ) ? 3 - ( WorkNr + CurrNr ) : PrevNr;
            WorkNr = CurrNr;
            CurrNr = NextNr;

        shared->camera_frame_ready.notify_one();
        shared->stats.camera_frames++;

        if ( !capture.isOpened() ) {
           std::this_thread::sleep_for( std::chrono::milliseconds(2000) ); 
        }

        if (shared->shutdown_requested.load())
        {
            break;
        }
    }
}
*/