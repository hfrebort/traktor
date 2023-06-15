//#include <functional>
#include <atomic>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "shared.h"
#include "detect.h"
#include "encode.h"
#include "pipeline/ImagePipeline.hpp"

static const std::vector<int> JPEGparams = {cv::IMWRITE_JPEG_QUALITY, 65};

const cv::Scalar RED   = cv::Scalar(  0,  0,255);
const cv::Scalar BLUE  = cv::Scalar(255,  0,  0);
const cv::Scalar GREEN = cv::Scalar(  0,255,  0);
const cv::Scalar BLACK = cv::Scalar(  0,  0,  0);
const cv::Scalar WHITE = cv::Scalar(255,255,255);

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
    static const cv::Scalar rowSpaceLineColor    (  0,  0,255);
 
    const int x_half = refSettings.x_half + refSettings.offset;
    //const int y_max  = imgSettings.frame_rows;
    const int y_max  = frame.rows();

    const int thresholdPx = refSettings.rowThresholdPx;
    const int spacePx     = refSettings.rowRangePx;
    const cv::Point Fluchtpunkt(x_half, -refSettings.rowPerspectivePx);
    
    cv::line(frame, cv::Point(x_half,0), cv::Point(x_half,y_max), rowLineColor, 2 );

    cv::line(frame, cv::Point(x_half-thresholdPx,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
    cv::line(frame, cv::Point(x_half+thresholdPx,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
    if ( spacePx > 0)
    {
        cv::line(frame, cv::Point((x_half - 0) - (thresholdPx + spacePx), y_max), Fluchtpunkt, rowSpaceLineColor, 1 );
        cv::line(frame, cv::Point((x_half - 0) + (thresholdPx + spacePx), y_max), Fluchtpunkt, rowSpaceLineColor, 1 );
    }

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
            cv::line(frame, cv::Point(x_half - x_spacing - thresholdPx ,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
            cv::line(frame, cv::Point(x_half - x_spacing + thresholdPx ,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
            cv::line(frame, cv::Point(x_half + x_spacing - thresholdPx ,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
            cv::line(frame, cv::Point(x_half + x_spacing + thresholdPx ,y_max), Fluchtpunkt, rowToleranceLineColor, 1 );
            // row "outer" spaces
            if ( spacePx > 0 )
            {
                cv::line(frame, cv::Point((x_half - x_spacing) - (thresholdPx + spacePx), y_max), Fluchtpunkt, rowSpaceLineColor, 1 );
                cv::line(frame, cv::Point((x_half - x_spacing) + (thresholdPx + spacePx), y_max), Fluchtpunkt, rowSpaceLineColor, 1 );
                cv::line(frame, cv::Point((x_half + x_spacing) - (thresholdPx + spacePx), y_max), Fluchtpunkt, rowSpaceLineColor, 1 );
                cv::line(frame, cv::Point((x_half + x_spacing) + (thresholdPx + spacePx), y_max), Fluchtpunkt, rowSpaceLineColor, 1 );
            }
        }
    }
}

static void draw_contoures_centers(cv::InputOutputArray frame, const DetectResult& detect_result)
{
    for ( Center plant : detect_result.contoures.centers )
    {
        const cv::Scalar* plant_color;
        if ( ! plant.within_row_range )
        {
            plant_color = &WHITE;
        }
        else if ( plant.within_threshold )
        {
            plant_color = &BLUE;
        }
        else
        {
            plant_color = &RED;
        }
        cv::drawMarker  ( frame, plant.point , *plant_color, cv::MarkerTypes::MARKER_CROSS, 20, 2 );
        cv::drawContours( frame, detect_result.contoures.all_contours, plant.contours_idx, *plant_color, 1 );
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
    auto overall_start = std::chrono::high_resolution_clock::now();

    if ( work->isPictureFromCamera )
    {
        auto start = overall_start;
        draw_row_lines        (work->frame, ctx->shared->detectSettings.getReflineSettings());
        draw_contoures_centers(work->frame, work->detect_result);
        draw_status_bar       (work->frame, work->detect_result, ctx->shared->detectSettings.getReflineSettings().x_half, ctx->status_bar);
        ctx->stats->draw += trk::get_duration(&start).count();
    }
    
    static const cv::String JPG(".jpg");
    uint64_t bytes_sent;

    if ( ! cv::imencode(JPG, work->frame, ctx->jpeg_buffer, JPEGparams) )
    {
        printf("E: error encoding frame to JPEG\n");
    }
    else if ( ! (ctx->sendJPEGbytes)(ctx->jpeg_buffer, &bytes_sent) )
    {
        puts("I: send of JPG failed. connection closed. quitting sending thread.");
        return WORKER_RC::LIKE_TO_EXIT;
    }
    else
    {
        ctx->stats->bytes_sent  += bytes_sent;
        ctx->stats->images_sent++;
    }

    ctx->stats->overall.fetch_add( trk::get_duration(&overall_start).count() );
    //printf("encode: overall: %lu\n", ctx->stats->overall.load());
    //printf("encode: bytes/images\t%lu/%lu\n", ctx->stats->bytes_sent.load(), ctx->stats->images_sent.load());

    return WORKER_RC::OK;
}
