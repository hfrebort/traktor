#include <atomic>
#include <optional>

#include <opencv2/opencv.hpp>

#include "shared.h"
#include "stats.h"
#include "util.h"

static const cv::Size GaussKernel(5,5);

const int erosion_size = 2;
const cv::Mat erodeKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size ( 2*erosion_size + 1, 2*erosion_size + 1 ),
                    cv::Point(   erosion_size    ,   erosion_size     ) );

const int dilate_size = 2;
const cv::Mat dilateKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size( 2*dilate_size + 1, 2*dilate_size + 1 ),
                    cv::Point( dilate_size, dilate_size ) );

cv::Mat tmp;
cv::Mat img_inRange;
cv::Mat img_GaussianBlur;
cv::Mat img_threshold;
cv::Mat img_eroded_dilated;

struct FoundStructures
{
    std::vector<cv::Point2i>    centers;
    std::vector<int>            matching_contours_idx;
    int                         YMax_center_idx;
    int                         YMax_contour_idx;

    void clear()
    {
        centers.clear();
        matching_contours_idx.clear();
        YMax_center_idx = -1;
        YMax_contour_idx = -1;
    }
};

std::vector< std::vector<cv::Point> >   g_contours;
FoundStructures                         g_structures;

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
}

void calc_centers(
      const std::vector< std::vector<cv::Point2i> >     &contours
    , const int                                          minimalContourArea
    , FoundStructures                                   *found)
{
    found->clear();

    int y_max = -1;
    cv::Point2i center;

    for ( int i=0; i < contours.size(); ++i )
    {
        const auto& contour = contours[i];
        if ( cv::contourArea(contour) > minimalContourArea )
        {
            found->matching_contours_idx.push_back(i);

            calc_center(contour, &center);
            found->centers.emplace_back( center.x, center.y );

            if ( center.y > y_max )
            {
                y_max = center.y;
                found->YMax_contour_idx = i;
                found->YMax_center_idx  = found->centers.size() - 1;
            }
        }
    }
}

void drawContoursAndCenters(
      cv::InputOutputArray                              frame
    , const std::vector< std::vector<cv::Point2i> >    &contours
    , const FoundStructures                            &found)
{
    for (int i=0; i < found.centers.size(); ++i)
    {
        const cv::Point2i &center = found.centers[i];
        if ( i == found.YMax_center_idx)
        {
            cv::drawMarker(frame, center, cv::Scalar(0,0,255), cv::MarkerTypes::MARKER_CROSS, 30, 3 );
        }
        else
        {
            cv::drawMarker(frame, center, cv::Scalar(255,0,0), cv::MarkerTypes::MARKER_CROSS, 20, 2 );
        }
    }

    for ( int contour_idx_to_draw : found.matching_contours_idx)
    {
        cv::drawContours( frame, contours, contour_idx_to_draw, cv::Scalar(255,0,0), 2 );
    }
}

void run_detection(cv::Mat& cameraFrame, const DetectSettings& settings, cv::Mat& outputFrame, Stats* stats, const bool showWindows)
{
    cv::Mat& buf1 = tmp;
    cv::Mat& buf2 = outputFrame;

    auto start = std::chrono::high_resolution_clock::now();

    cv::cvtColor    (cameraFrame, buf2, cv::COLOR_BGR2HSV );
    cv::inRange     (buf2, settings.colorFrom, settings.colorTo, buf1 );                        if (showWindows) { buf1.copyTo(img_inRange); }
    cv::GaussianBlur(buf1, buf2, GaussKernel, 0);                                               if (showWindows) { buf2.copyTo(img_GaussianBlur); }
    cv::threshold   (buf2, buf1, 0, 255, cv::THRESH_BINARY );                                   if (showWindows) { buf1.copyTo(img_threshold);}
    cv::erode       (buf1, buf2, erodeKernel, cv::Point(-1,-1), settings.erode_iterations  );
    cv::dilate      (buf2, buf1, dilateKernel,cv::Point(-1,-1), settings.dilate_iterations );   if (showWindows) { buf1.copyTo(img_eroded_dilated); }

    stats->prepare_ns += trk::getDuration_ns(&start);
    
    cv::findContours(buf1, g_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    stats->findContours_ns += trk::getDuration_ns(&start);

    int idx_Ymax_contour;
    calc_centers(g_contours, settings.minimalContourArea, &g_structures);
    stats->calcCenters_ns += trk::getDuration_ns(&start);
    
    cameraFrame.copyTo(outputFrame);
    drawContoursAndCenters(outputFrame, g_contours, g_structures );
    stats->draw_ns += trk::getDuration_ns(&start);

    std::optional<int> offset_px;
    if ( g_structures.YMax_center_idx > -1 )
    {
        auto YMax_point = g_structures.centers[ g_structures.YMax_center_idx ];
        offset_px = YMax_point.x - ( cameraFrame.cols / 2 );
    }

    if (showWindows)
    {
        cv::imshow("inRange",       img_inRange );
        cv::imshow("GaussianBlur",  img_GaussianBlur );
        cv::imshow("threshold",     img_threshold );
        cv::imshow("eroded_dilated",img_eroded_dilated );
        cv::imshow("drawContours",  outputFrame );
        cv::waitKey(1);
    }
}

void thread_detect(Shared* shared, Stats* stats)
{
    printf("I: thread detect running\n");

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
        //
        // lock an output buffer
        //
        {
            std::lock_guard<std::mutex> lk(shared->analyzed_frame_buf_mutex[idx_doubleBuffer]);
            run_detection(
                  cameraFrame                                       // input
                , shared->detectSettings                            // schieberegler
                , shared->analyzed_frame_buf[idx_doubleBuffer]      // output
                , stats
                , true );                                           // show opencv windows with img processing Zwischensteps
        }
        stats->fps++;
        //
        // set index for other thread
        //
        shared->analyzed_frame_ready_idx.store(idx_doubleBuffer);
        //
        // notify other thread about ready buffer
        // 2021-08-08 Spindler (Moz'ens Geburtstag)
        //   Still don't know if a lock is required to do .notify_xxx()
        //
        {
            std::unique_lock<std::mutex> ul(shared->analyzed_frame_ready_mutex);
            shared->analyzed_frame_ready.notify_one();
        }
        
        idx_doubleBuffer = 1 - idx_doubleBuffer;

        if (shared->shutdown_requested.load())
        {
            printf("I: thread detect quitting...\n");
            break;
        }
    }
}