#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

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
        ctx->capture = std::make_unique<cv::VideoCapture>();
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
    work->isPictureFromCamera = ( ctx->errorCount == 0 );
}
