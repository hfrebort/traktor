//#include <functional>
//#include <atomic>
//#include <opencv2/opencv.hpp>
//#include <opencv2/imgcodecs.hpp>

#include "shared.h"
#include "detect.h"
#include "encode.h"
#include "pipeline/ImagePipeline.hpp"

static const std::vector<int> JPEGparams = {cv::IMWRITE_JPEG_QUALITY, 65};

const cv::Scalar RED   = cv::Scalar(  0,  0,255);
const cv::Scalar BLUE  = cv::Scalar(255,  0,  0);
const cv::Scalar GREEN = cv::Scalar(  0,255,  0);
const cv::Scalar BLACK = cv::Scalar(  0,  0,  0);

/***
 * 2021-08-30 Spindler
 *   the lines left and right:
 *      start point: botton of the frame. +/- x_spacing
 *      end   point: Fluchtpunkt(!) des Bildes. X ist mittig. Y ist ein Wert "oberhalb" des Bildes. Minus Y!
 ***/
static void draw_row_lines(cv::InputOutputArray frame, const ReflinesSettings& refSettings)
{
    static const cv::Scalar rowLineColor         (150,255,255);
    static const cv::Scalar rowToleranceLineColor( 60,255,128);

    const int x_half = refSettings.x_half;
    //const int y_max  = imgSettings.frame_rows;
    const int y_max  = frame.rows();

    const int       deltapx = refSettings.rowThresholdPx;
    const cv::Point Fluchtpunkt(x_half, -refSettings.rowPerspectivePx);
    
    cv::line(frame, cv::Point(x_half,0), cv::Point(x_half,y_max), rowLineColor, 2 );

    cv::line(frame, cv::Point(x_half-deltapx,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
    cv::line(frame, cv::Point(x_half+deltapx,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );

    //
    // Reihen rechts und links der Mittellinie zeichnen
    //
    if ( refSettings.get_half_row_count() > 0 )
    {
        for ( int r=refSettings.get_half_row_count(), x_spacing = refSettings.rowSpacingPx; 
              r > 0; 
              r-=1, x_spacing += refSettings.rowSpacingPx )
        {
            // rows
            cv::line(frame, cv::Point(x_half - x_spacing,y_max),            Fluchtpunkt, rowLineColor, 2 );
            cv::line(frame, cv::Point(x_half + x_spacing,y_max),            Fluchtpunkt, rowLineColor, 2 );
            // row tolerance
            cv::line(frame, cv::Point(x_half - x_spacing - deltapx ,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
            cv::line(frame, cv::Point(x_half - x_spacing + deltapx ,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
            cv::line(frame, cv::Point(x_half + x_spacing - deltapx ,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
            cv::line(frame, cv::Point(x_half + x_spacing + deltapx ,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
        }
    }
}

static void draw_contoures_centers(cv::InputOutputArray frame, const DetectResult& detect_result)
{
    for ( Center plant : detect_result.contoures.centers )
    {
        const cv::Scalar& plant_color = plant.within_threshold ? BLUE : RED;
        cv::drawMarker  ( frame, plant.point , plant_color, cv::MarkerTypes::MARKER_CROSS, 20, 2 );
        cv::drawContours( frame, detect_result.contoures.all_contours, plant.contours_idx, plant_color, 1 );
    }
}

static void init_status_bar(const cv::Mat& frame, std::unique_ptr<cv::Mat>& status_bar)
{
    if ( status_bar == nullptr )
    {
        status_bar = std::make_unique<cv::Mat>( cv::Mat::zeros(20, frame.cols, frame.type() ) );
    }
    else if ( status_bar->cols != frame.cols ) {
        status_bar.reset();
        status_bar = std::make_unique<cv::Mat>( cv::Mat::zeros(20, frame.cols, frame.type() ) );
    }
    status_bar->setTo( BLACK );
}

static void write_text_to_status_bar(const cv::String& text, cv::Mat* bar) 
{
    cv::putText(*bar, text, cv::Point(10, 19), cv::FONT_HERSHEY_SIMPLEX, 0.8, RED, 2);
}

static void draw_threshold_bar(const bool within_threshold, const float avg_threshold, const int x_half, cv::Mat* bar)
{
    const cv::Scalar& color_overall_delta = within_threshold ? GREEN : RED;
    const int delta_status_px = (float)avg_threshold * (float)x_half;
    cv::rectangle(*bar, cv::Point(x_half,0), cv::Point(x_half + delta_status_px,bar->rows), color_overall_delta, cv::FILLED);
}

static void draw_status_bar(cv::Mat& frame, const DetectResult& detect_result, const int x_half, std::unique_ptr<cv::Mat>& status_bar)
{
    init_status_bar(frame, status_bar);

    if ( detect_result.state == DETECT_STATE::HARROW_LIFTED )
    {
        write_text_to_status_bar("LIFTED", status_bar.get());
    }
    else if ( detect_result.state == DETECT_STATE::NO_PLANTS_WITHIN_LINES )
    {
        write_text_to_status_bar("NO PLANTS WITHIN LINES", status_bar.get());
    }
    else if ( detect_result.state == DETECT_STATE::NOTHING_FOUND )
    {
        write_text_to_status_bar("NOTHING FOUND", status_bar.get());
    }
    else if ( detect_result.state == DETECT_STATE::NO_VALID_FRAME )
    {
        write_text_to_status_bar("NO VALID FRAME", status_bar.get());
    }
    else if ( detect_result.state == DETECT_STATE::SUCCESS )
    {
        draw_threshold_bar(detect_result.is_in_threshold, detect_result.avg_threshold, x_half, status_bar.get());
    }

    frame.push_back(*status_bar);
}

WORKER_RC encode_main(Workitem* work, EncodeContext* ctx)
{
    if ( work->isValidForAnalyse )
    {
        draw_row_lines        (work->frame, ctx->shared->detectSettings.getReflineSettings());
        draw_contoures_centers(work->frame, work->detect_result);
        draw_status_bar       (work->frame, work->detect_result, ctx->shared->detectSettings.getReflineSettings().x_half, ctx->status_bar);
    }
    
    static const cv::String JPG(".jpg");
    if ( ! cv::imencode(JPG, work->frame, ctx->jpeg_buffer, JPEGparams) )
    {
        printf("E: error encoding frame to JPEG\n");
    }
    else if ( ! (ctx->sendJPEGbytes)(ctx->jpeg_buffer) )
    {
        puts("I: send of JPG failed. connection closed. quitting sending thread.");
        return WORKER_RC::LIKE_TO_EXIT;
    }
    return WORKER_RC::OK;
}
/*
void thread_send_jpeg(Shared* shared, std::function<bool(std::vector<uchar>&)> sendJPEGbytes)
{
    const std::vector<int> JPEGparams = {cv::IMWRITE_JPEG_QUALITY, 65};

    shared->stats.jpeg_threads_running.fetch_add(1);
    printf("I: thread send_jpeg started. threadcount: %d\n", shared->stats.jpeg_threads_running.load());

    short idx = -1;

    for (;;)
    {
        //
        // wait to be signaled by detection thread for a new frame
        //
        {
            std::unique_lock<std::mutex> ul(shared->analyzed_frame_ready_mutex);
            shared->analyzed_frame_ready.wait(ul, [&idx,&shared]() { return idx != shared->analyzed_frame_ready_idx.load(); });
        }
        //
        // load new idx of buffer
        //
        idx = shared->analyzed_frame_ready_idx.load();
        
        bool encoded;
        {
            //
            // lock the specific buffer for the duration of the encoding
            //
            std::lock_guard<std::mutex> lk(shared->analyzed_frame_buf_mutex[idx]);
            if ( shared->analyzed_frame_encoded_to_JPEG.load() == false)
            {
                encoded = cv::imencode(".jpg", shared->analyzed_frame_buf[idx], shared->analyzed_frame_jpegImage, JPEGparams);
                shared->analyzed_frame_encoded_to_JPEG.store(true);
            }
            else
            {
                encoded = true;
            }
        }
        if ( !encoded )
        {
            printf("E: error encoding frame to JPEG\n");
        }
        else
        {
            if ( !sendJPEGbytes(shared->analyzed_frame_jpegImage) )
            {
                puts("I: send of JPG failed. connection closed. quitting sending thread.");
                break;
            }
            shared->stats.jpeg_sent++;
        }

        if ( shared->shutdown_requested.load() )
        {
            break;
        }
    }
    shared->stats.jpeg_threads_running.fetch_add(-1);
    printf("I: A send_jpeg thread (webserver) ended. threadcount: %d\n", shared->stats.jpeg_threads_running.load());
}
*/