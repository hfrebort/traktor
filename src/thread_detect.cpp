//#include <atomic>
//#include <optional>
//
////#include <opencv2/opencv.hpp>
//#include <opencv2/core.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui.hpp>
//
#include "shared.h"
#include "stats.h"
#include "util.h"
//
static const cv::Size GaussKernel(5,5);

const int erosion_size = 2;
const cv::Mat erodeKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size ( 2*erosion_size + 1, 2*erosion_size + 1 ),
                    cv::Point(   erosion_size    ,   erosion_size     ) );

const int dilate_size = 2;
const cv::Mat dilateKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size( 2*dilate_size + 1, 2*dilate_size + 1 ),
                    cv::Point( dilate_size, dilate_size ) );

const cv::Scalar RED  = cv::Scalar(0,0,255);
const cv::Scalar BLUE = cv::Scalar(255,0,0);

cv::Mat tmp;
cv::Mat img_inRange;
cv::Mat img_GaussianBlur;
cv::Mat img_threshold;
cv::Mat img_eroded_dilated;

struct Structures
{
    std::vector< std::vector<cv::Point> >   all_contours;
    std::vector<cv::Point2i>                centers;
    std::vector<int>                        centers_contours_idx;

    void clearCenters()
    {
        centers.clear();
        centers_contours_idx.clear();
    }
};

/* 2021-08-23 Spindler
    nice try but doesn't work as expected
void calc_center(const std::vector<cv::Point2i>& points, cv::Point* center)
{
    size_t sum_x = 0;
    size_t sum_y = 0;

    for ( const auto& p : points )
    {
        sum_x += p.x;
        sum_y += p.y;
    }

    center->x = sum_x / points.size();
    center->y = sum_y / points.size();
}*/

void calc_center2(const std::vector<cv::Point2i>& points, cv::Point* center)
{
    cv::Moments M = cv::moments(points);
    center->x = int(M.m10 / M.m00);
	center->y = int(M.m01 / M.m00);
}

void calc_centers(Structures* found, const int minimalContourArea)
{
    found->clearCenters();

    cv::Point2i centerPoint;

    for ( int i=0; i < found->all_contours.size(); ++i )
    {
        const auto& contour = found->all_contours[i];
        if ( cv::contourArea(contour) > minimalContourArea )
        {
            //
            // calc center and add it to centers
            //
            calc_center2(contour, &centerPoint);
            found->centers.emplace_back( centerPoint.x, centerPoint.y );
            //
            // remeber index of found center to access the corresponding contour afterwards
            //
            found->centers_contours_idx.push_back(i);
        }
    }
}

void drawContoursAndCenters(cv::InputOutputArray frame, const DetectSettings &settings, const Structures &found)
{
    for ( auto const& c : found.centers )
    {
        cv::drawMarker(frame, c, BLUE, cv::MarkerTypes::MARKER_TILTED_CROSS, 20, 2 );
    }

    for ( int contour_idx_to_draw : found.centers_contours_idx)
    {
        cv::drawContours( frame, found.all_contours, contour_idx_to_draw, BLUE, 2 );
    }
}

/***
 * 2021-08-30 Spindler
 *   the lines left and right:
 *      start point: botton of the frame. +/- x_spacing
 *      end   point: Fluchtpunkt(!) des Bildes. X ist mittig. Y ist ein Wert "oberhalb" des Bildes. Minus Y!
 ***/
void drawRowLines(cv::InputOutputArray frame, const DetectSettings &settings)
{
    cv::Scalar rowLineColor(150,255,255);
    //
    // Mittn
    //
    int x_half = settings.frame_cols / 2;
    int y_max  = settings.frame_rows;
    
    cv::line(frame, cv::Point(x_half,0), cv::Point(x_half,y_max), rowLineColor, 2 );
    //
    // Reihen rechts und links der Mittellinie zeichnen
    //
    if ( settings.rowCount > 1 )
    {
        for ( int r=settings.rowCount-1, x_spacing = settings.rowSpacePx; r > 0; r-=2, x_spacing += settings.rowSpacePx )
        {
            cv::line(frame, cv::Point(x_half - x_spacing,y_max), cv::Point(x_half, -settings.rowPerspectivePx ), rowLineColor, 2 );
            cv::line(frame, cv::Point(x_half + x_spacing,y_max), cv::Point(x_half, -settings.rowPerspectivePx ), rowLineColor, 2 );
        }
    }
}

void find_contours(cv::Mat& cameraFrame, const DetectSettings& settings, cv::Mat& outputFrame, Structures* structures, Stats* stats, const bool showWindows)
{
    cv::Mat* in  = &tmp;
    cv::Mat* out = &outputFrame;

    auto start = std::chrono::high_resolution_clock::now();

    cv::cvtColor    (cameraFrame, *out, cv::COLOR_BGR2HSV );                                 std::swap(in,out);
    cv::GaussianBlur(*in, *out, GaussKernel, 0);                                             std::swap(in,out); if (showWindows) { in->copyTo(img_GaussianBlur); }
    cv::inRange     (*in, settings.colorFrom, settings.colorTo, *out );                      std::swap(in,out); if (showWindows) { in->copyTo(img_inRange); }
    //cv::threshold   (*in, *out, 0, 255, cv::THRESH_BINARY );                                 std::swap(in,out); if (showWindows) { in->copyTo(img_threshold);}
    cv::erode       (*in, *out, erodeKernel, cv::Point(-1,-1), settings.erode_iterations  ); std::swap(in,out);
    cv::dilate      (*in, *out, dilateKernel,cv::Point(-1,-1), settings.dilate_iterations );                    if (showWindows) { out->copyTo(img_eroded_dilated); }

    stats->prepare_ns += trk::getDuration_ns(&start);
    
    cv::findContours(*out, structures->all_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);       stats->findContours_ns  += trk::getDuration_ns(&start);
}

void thread_detect(Shared* shared, Stats* stats, bool showDebugWindows)
{
    printf("I: thread detect running\n");

    Structures structures;
    int idx_doubleBuffer = 0;

    for (;;)
    {
        //
        // get new frame from camera buffer
        //
        int frameReadyIdx;
        while ( (frameReadyIdx=std::atomic_exchange( &(shared->frame_buf_slot), -1)) == -1 )
        {
            // wait for a frame ready in the frameBuffer
            std::unique_lock<std::mutex> lk(shared->camera_frame_ready_mutex);
            shared->camera_frame_ready.wait(lk, [&shared]() { return shared->frame_buf_slot.load() != -1; });
        }
        cv::Mat& cameraFrame = shared->frame_buf[frameReadyIdx];
        cv::Mat& outFrame    = shared->analyzed_frame_buf[idx_doubleBuffer];
        //
        // lock an output buffer
        //
        {
            std::lock_guard<std::mutex> lk(shared->analyzed_frame_buf_mutex[idx_doubleBuffer]);
            //
            // 1. detect
            //
            auto start = std::chrono::high_resolution_clock::now();
            find_contours(
                  cameraFrame               
                , shared->detectSettings    
                , outFrame                  
                , &structures
                , stats
                , showDebugWindows );                               
            stats->detect_overall_ns += trk::getDuration_ns(&start);
            calc_centers(&structures, shared->detectSettings.minimalContourArea);  stats->calcCenters_ns += trk::getDuration_ns(&start);
            //
            // draw in picture
            //
            cameraFrame.copyTo(outFrame);
            
            drawContoursAndCenters(outFrame, shared->detectSettings, structures );
            drawRowLines          (outFrame, shared->detectSettings );

            stats->draw_ns += trk::getDuration_ns(&start);
        }
        if (showDebugWindows)
        {
            cv::imshow("inRange",       img_inRange );
            cv::imshow("GaussianBlur",  img_GaussianBlur );
            //cv::imshow("threshold",     img_threshold );
            cv::imshow("eroded_dilated",img_eroded_dilated );
            cv::imshow("drawContours",  shared->analyzed_frame_buf[idx_doubleBuffer] );
            cv::waitKey(1);
        }
        stats->fps++;
        //
        // set index for other thread
        //
        shared->analyzed_frame_ready_idx.store(idx_doubleBuffer);
        //
        // notify other thread(s) about ready buffer
        // 2021-08-08 Spindler (Moz'ens Geburtstag)
        //   Still don't know if a lock is required to do .notify_xxx()
        //
        {
            std::unique_lock<std::mutex> ul(shared->analyzed_frame_ready_mutex);
            shared->analyzed_frame_encoded_to_JPEG.store(false);
            shared->analyzed_frame_ready.notify_all();
        }
        
        idx_doubleBuffer = 1 - idx_doubleBuffer;

        if (shared->shutdown_requested.load())
        {
            printf("I: thread detect quitting...\n");
            break;
        }
    }
}