#include <atomic>
#include <opencv2/opencv.hpp>

#include "shared.h"
#include "stats.h"

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
std::vector< std::vector<cv::Point> > contours; 

void calc_center(const std::vector<cv::Point2i>& points, cv::Point* center)
{
    size_t sum_x = 0;
    size_t sum_y = 0;

    for ( auto& p : points )
    {
        sum_x += p.x;
        sum_y += p.y;
    }

    center->x = sum_x / points.size();
    center->y = sum_y / points.size();
}

void drawContoursAndCenters(cv::InputOutputArray frame, const std::vector< std::vector<cv::Point2i> >& contours)
{
    cv::drawContours(frame, contours, -1, cv::Scalar( 0, 0, 255 ) );

    cv::Point2i center;
    for ( auto& contour : contours )
    {
        calc_center(contour, &center);
        cv::drawMarker(frame, center, cv::Scalar(255,0,0), cv::MarkerTypes::MARKER_CROSS, 20, 2 );
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

    auto finish = std::chrono::high_resolution_clock::now();
    stats->prepare += std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count();

    cv::findContours(buf1, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cameraFrame.copyTo(outputFrame);
    drawContoursAndCenters(outputFrame, contours);
    
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
                , true );
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