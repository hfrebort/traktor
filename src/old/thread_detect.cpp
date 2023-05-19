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
#include "harrow.h"
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

const cv::Scalar RED   = cv::Scalar(0,0,255);
const cv::Scalar BLUE  = cv::Scalar(255,0,0);
const cv::Scalar GREEN = cv::Scalar(0,255,0);

cv::Mat tmp;
cv::Mat img_inRange;
cv::Mat img_GaussianBlur;
cv::Mat img_threshold;
cv::Mat img_eroded_dilated;

struct Contoures
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

size_t mat_byte_size(const cv::Mat& mat)
{
    return mat.total() * mat.elemSize();
}

void calc_center_of_contour(const std::vector<cv::Point2i>& points, cv::Point* center)
{
    cv::Moments M = cv::moments(points);
    center->x = int(M.m10 / M.m00);
	center->y = int(M.m01 / M.m00);
}

void drawContoursAndCenters(cv::InputOutputArray frame, const DetectSettings &settings, const Contoures &found)
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
void drawRowLines(cv::InputOutputArray frame, const ImageSettings &imgSettings, const ReflinesSettings& refSettings)
{
    const cv::Scalar rowLineColor         (150,255,255);
    const cv::Scalar rowToleranceLineColor( 60,255,128);

    const int x_half = refSettings.x_half;
    const int y_max  = imgSettings.frame_rows;

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

//cv::threshold   (*in, *out, 0, 255, cv::THRESH_BINARY );                                 std::swap(in,out); if (showWindows) { in->copyTo(img_threshold);}        stats->frame_bytes_processed += mat_byte_size(*in);
void find_contours(cv::Mat& cameraFrame, const ImageSettings& settings, cv::Mat& outputFrame, Contoures* structures, Stats* stats, const bool showWindows)
{
    cv::Mat* in  = &tmp;
    cv::Mat* out = &outputFrame;

    auto start = std::chrono::high_resolution_clock::now();

    auto prep_start = start;
    cv::cvtColor    (cameraFrame, *out, cv::COLOR_BGR2HSV );                                 stats->prepare_cvtColor_ns     += trk::getDuration_ns(&prep_start);   std::swap(in,out);                                                         stats->frame_bytes_processed += mat_byte_size(cameraFrame);
    cv::GaussianBlur(*in, *out, GaussKernel, 0);                                             stats->prepare_GaussianBlur_ns += trk::getDuration_ns(&prep_start);   std::swap(in,out); if (showWindows) { in->copyTo(img_GaussianBlur); }      stats->frame_bytes_processed += mat_byte_size(*in);
    cv::inRange     (*in, settings.colorFrom, settings.colorTo, *out );                      stats->prepare_inRange_ns      += trk::getDuration_ns(&prep_start);   std::swap(in,out); if (showWindows) { in->copyTo(img_inRange); }           stats->frame_bytes_processed += mat_byte_size(*in);
    cv::erode       (*in, *out, erodeKernel, cv::Point(-1,-1), settings.erode_iterations  ); stats->prepare_erode_ns        += trk::getDuration_ns(&prep_start);   std::swap(in,out);                                                         stats->frame_bytes_processed += mat_byte_size(*in);
    cv::dilate      (*in, *out, dilateKernel,cv::Point(-1,-1), settings.dilate_iterations ); stats->prepare_dilate_ns       += trk::getDuration_ns(&prep_start);                      if (showWindows) { out->copyTo(img_eroded_dilated); }   stats->frame_bytes_processed += mat_byte_size(*in);

    stats->prepare_ns += trk::getDuration_ns(&start);
    
    cv::findContours(*out, structures->all_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);       stats->findContours_ns  += trk::getDuration_ns(&start);         stats->frame_bytes_processed += mat_byte_size(*out);
}

void calc_centers_of_contours(Contoures* found, const int minimalContourArea)
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
            calc_center_of_contour(contour, &centerPoint);
            found->centers.emplace_back( centerPoint.x, centerPoint.y );
            //
            // remeber index of found center to access the corresponding contour afterwards
            //
            found->centers_contours_idx.push_back(i);
        }
    }
}
/*
 * y = x * ( 10 / 2 )   
 * x = y / ( 10 / 2 )
 * Wolfram Alpha: plot [y = (-x+2) * (10/2), {x,-2,2}]
 */
bool find_point_on_nearest_refline(
      const cv::Point&        plant
    , const ReflinesSettings& settings
    , float  *nearest_refLine_x
    , float  *deltaPx
    , float  *refLines_distance_px)
{
    const float plant_x_abs_offset_from_0  = std::abs<int>( plant.x );

    float px_delta_to_row;
    float x_ref1 = 0;    // start with the middle
    float x_ref2;

    int refline_x = settings.rowSpacingPx;
    for ( int r=0; r < settings.get_half_row_count(); ++r, refline_x += settings.rowSpacingPx)
    {
        const float refline_steigung = settings.y_fluchtpunkt / (float)refline_x;
        x_ref2 = -plant.y / refline_steigung ;
        x_ref2 +=  refline_x;

        if ( x_ref1 < plant_x_abs_offset_from_0 && plant_x_abs_offset_from_0 < x_ref2 )
        {
            // plant is between rows. which row is closer?
            const float delta_to_row1_px = plant_x_abs_offset_from_0 - x_ref1;
            const float delta_to_row2_px = x_ref2 - plant_x_abs_offset_from_0;

            if ( delta_to_row1_px < delta_to_row2_px )
            {
                *nearest_refLine_x = x_ref1;
                *deltaPx = delta_to_row1_px;        // row 1 is left from the plant. delta is positive.
            }
            else
            {
                *nearest_refLine_x = x_ref2;
                *deltaPx = -delta_to_row2_px;       // row 2 is right from the plant. delta is negative.
            }

            *refLines_distance_px = x_ref2 - x_ref1;
            assert(*refLines_distance_px > 0);

            if ( plant.x < 0 )
            {
                // plant is on the left side. Mirror the value
                *nearest_refLine_x = -*nearest_refLine_x;
                *deltaPx = -*deltaPx;
            }
            // shift it to the right pixel-image-value
            *nearest_refLine_x += settings.x_half;

            return true;
        }
        else
        {
            // try next rows
            x_ref1 = x_ref2;
        }
    }
    
    return false;
}

void draw_status_bar(const cv::String& text, cv::Mat* bar) 
{
    cv::putText(*bar, text, cv::Point(10, 19), cv::FONT_HERSHEY_SIMPLEX, 0.8, RED, 2);
}

void draw_threshold_bar(const bool within_threshold, const float avg_threshold, const int x_half, cv::Mat* bar)
{
    const cv::Scalar& color_overall_delta = within_threshold ? GREEN : RED;
    const int delta_status_px = (float)avg_threshold * (float)x_half;
    cv::rectangle(*bar, cv::Point(x_half,0), cv::Point(x_half + delta_status_px,bar->rows), color_overall_delta, cv::FILLED);
}

bool is_within_threshold(const float avg_threshold, const int rowSpacingPx, const int rowThresholdPx)
{
    const int x_overall_threshold_px = (float)avg_threshold * (float)rowSpacingPx;
    const bool is_within_threshold = std::abs(x_overall_threshold_px) < rowThresholdPx;

    return is_within_threshold;
}

HARROW_DIRECTION get_harrow_direction(const bool is_within_threshold, const float avg_threshold)
{
    HARROW_DIRECTION direction;

    if ( is_within_threshold ) {
        direction = HARROW_DIRECTION::STOP;
    }
    else if ( avg_threshold > 0 ) {
        direction = HARROW_DIRECTION::RIGHT;
    }
    else {
        direction = HARROW_DIRECTION::LEFT;
    }

    return direction;
}

bool calc_overall_threshold_draw_plants(Contoures* structures, DetectSettings& settings, cv::Mat& frame, float* avg_threshold)
{
    const ReflinesSettings& refSettings = settings.getReflineSettings();
    const ImageSettings&    imgSettings = settings.getImageSettings();

    const float threshold_percent = (float)refSettings.rowThresholdPx / (float)refSettings.rowSpacingPx;

    float   sum_threshold = 0;
    cv::Point plant_coord;

    uint structures_processed = 0;
    for ( int i=0; i < structures->centers.size(); ++i )
    {
        const cv::Point& plant = structures->centers[i];
        // hier bitte mit Magie befüllen!

        plant_coord.x = plant.x - refSettings.x_half;
        plant_coord.y = imgSettings.frame_rows - plant.y;

        float nearest_refLine_x;
        float deltaPx;
        float refLines_distance_px;
        if ( find_point_on_nearest_refline(plant_coord, refSettings, &nearest_refLine_x, &deltaPx, &refLines_distance_px) )
        {
            const float threshold = deltaPx / refLines_distance_px;
            sum_threshold += threshold;
            
            const cv::Scalar& plant_color = std::abs(threshold) < threshold_percent ? BLUE : RED;
            cv::drawMarker  ( frame, plant , plant_color, cv::MarkerTypes::MARKER_CROSS, 20, 2 );
            cv::drawContours( frame, structures->all_contours, structures->centers_contours_idx[i], plant_color, 1 );
            structures_processed += 1;
        }
    }
    if ( structures_processed > 0 ) {
        *avg_threshold = sum_threshold / (float)structures_processed;
    }

    return structures_processed > 0;
}



void thread_detect(Shared* shared, Stats* stats, Harrow* harrow, bool showDebugWindows)
{
    try
    {
        printf("I: thread detect running\n");

        Contoures structures;
        int idx_doubleBuffer = 0;

        const DetectSettings   &detectSettings  = shared->detectSettings;
        const ImageSettings    &imageSettings   = shared->detectSettings.getImageSettings();
        const ReflinesSettings &reflineSettings = shared->detectSettings.getReflineSettings();
        
        std::unique_ptr<cv::Mat> status_bar = nullptr;

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
            // status_bar
            //
            if ( status_bar == nullptr )
            {
                status_bar = std::make_unique<cv::Mat>( cv::Mat::zeros(20, cameraFrame.cols, cameraFrame.type() ) );
            }
            else if ( status_bar->cols != cameraFrame.cols ) {
                status_bar.reset();
                status_bar = std::make_unique<cv::Mat>( cv::Mat::zeros(20, cameraFrame.cols, cameraFrame.type() ) );
            }
            //
            // lock an output buffer
            //
            HARROW_DIRECTION direction = HARROW_DIRECTION::STOP;
            const bool detecting = detectSettings.detecting.load();
            const bool harrow_lifted = shared->harrowLifted.load();
            {
                std::lock_guard<std::mutex> lk(shared->analyzed_frame_buf_mutex[idx_doubleBuffer]);
                auto overallstart = std::chrono::high_resolution_clock::now();

                status_bar->setTo( cv::Scalar(0,0,0) );

                if ( harrow_lifted )
                {
                    draw_status_bar("LIFTED", status_bar.get() );
                    cameraFrame.copyTo(outFrame);
                }
                else
                {
                    find_contours(
                        cameraFrame               
                        , imageSettings   
                        , outFrame                  
                        , &structures
                        , stats
                        , showDebugWindows );                                                           

                    auto start = std::chrono::high_resolution_clock::now();
                    cameraFrame.copyTo(outFrame);
                    stats->copyTo_ns    += trk::getDuration_ns(&start);
                    stats->copyTo_bytes += mat_byte_size(cameraFrame);
                    //printf("cameraFrame size: %lu\n", mat_byte_size(cameraFrame));

                    calc_centers_of_contours(&structures, imageSettings.minimalContourArea);

                    float avg_threshold;
                    if ( !detecting )
                    {
                        draw_status_bar("DETECTION OFF", status_bar.get());
                    }
                    else if ( structures.centers.size() == 0 )
                    {
                        draw_status_bar("NOTHING FOUND", status_bar.get());
                    }
                    else if ( ! calc_overall_threshold_draw_plants(&structures, shared->detectSettings, outFrame, &avg_threshold) )
                    {
                        draw_status_bar("NO PLANTS WITHIN LINES", status_bar.get());
                    }
                    else
                    {
                        const bool is_in_threshold = is_within_threshold(avg_threshold, reflineSettings.rowSpacingPx, reflineSettings.rowThresholdPx);
                        direction = get_harrow_direction(is_in_threshold, avg_threshold);
                        draw_threshold_bar(is_in_threshold, avg_threshold, reflineSettings.x_half, status_bar.get());
                    }
                    drawRowLines(outFrame, imageSettings, reflineSettings);
                    stats->calc_draw_ns += trk::getDuration_ns(&start);
                }
                outFrame.push_back(*status_bar);
                stats->detect_overall_ns += trk::getDuration_ns(&overallstart);
            }
            if (showDebugWindows)
            {
                cv::imshow("inRange",       img_inRange );
                cv::imshow("GaussianBlur",  img_GaussianBlur );
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
            // 2022-03-31 Spindler (Sternzeit: -300754.4210426179)
            //  hob jetzt no amoi nochgschaut: https://en.cppreference.com/w/cpp/thread/condition_variable
            //  "3. execute notify_one or notify_all on the std::condition_variable (the lock does not need to be held for notification)"
            //
            {
                std::unique_lock<std::mutex> ul(shared->analyzed_frame_ready_mutex);
                shared->analyzed_frame_encoded_to_JPEG.store(false);
                shared->analyzed_frame_ready.notify_all();
            }
            //
            // kontroll se Hacke
            //
            if (       harrow         != nullptr 
                    && harrow_lifted  == false 
                    && detecting      == true)
            {
                harrow->move(direction, "detect");
            }
            
            idx_doubleBuffer = 1 - idx_doubleBuffer;

            if (shared->shutdown_requested.load())
            {
                break;
            }
        }
    }
    catch(const std::exception& e)
    {
        fprintf(stderr,"X: detect - %s\n", e.what());
        shared->shutdown_requested.store(true);
    }
}
