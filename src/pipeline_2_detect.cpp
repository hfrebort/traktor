#include <opencv2/imgproc.hpp>

#include "detect.h"
#include "pipeline/ImagePipeline.hpp"

void init_status_bar(int frame_columns, int frame_type, std::unique_ptr<cv::Mat>& status_bar)
{
    if ( status_bar == nullptr )
    {
        status_bar = std::make_unique<cv::Mat>( cv::Mat::zeros(20, frame_columns, frame_type ) );
    }
    else if ( status_bar->cols != frame_columns ) {
        status_bar.reset();
        status_bar = std::make_unique<cv::Mat>( cv::Mat::zeros(20, frame_columns, frame_type ) );
    }
}

static const cv::Size GaussKernel(5,5);

const int erosion_size = 2;
const cv::Mat erodeKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size ( 2*erosion_size + 1, 2*erosion_size + 1 ),
                    cv::Point(   erosion_size    ,   erosion_size     ) );

const int dilate_size = 2;
const cv::Mat dilateKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size( 2*dilate_size + 1, 2*dilate_size + 1 ),
                    cv::Point( dilate_size, dilate_size ) );

const cv::Scalar RED    = cv::Scalar(0,0,255);
const cv::Scalar BLUE   = cv::Scalar(255,0,0);
const cv::Scalar GREEN  = cv::Scalar(0,255,0);
const cv::Point  POINT_MINUS1 = cv::Point(-1,-1);

size_t mat_byte_size(const cv::Mat& mat)
{
    return mat.total() * mat.elemSize();
}

void find_contours(const ImageSettings& settings, DetectCounter* stats, const cv::Mat& cameraFrame, Contoures* structures, const bool showWindows)
{
    static cv::Mat img_inRange;
    static cv::Mat img_GaussianBlur;
    static cv::Mat img_eroded_dilated;

    static cv::Mat in;
    static cv::Mat out;

    auto start = std::chrono::high_resolution_clock::now();

    cv::cvtColor    (cameraFrame, out, cv::COLOR_BGR2HSV );                                      stats->cvtColor     += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(cameraFrame);   std::swap(in,out);                                                      
    cv::GaussianBlur(in, out, GaussKernel, 0);                                                   stats->GaussianBlur += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(in);            std::swap(in,out); if (showWindows) { in.copyTo(img_GaussianBlur); }    
    cv::inRange     (in, settings.colorFrom, settings.colorTo, out );                            stats->inRange      += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(in);            std::swap(in,out); if (showWindows) { in.copyTo(img_inRange); }         
    cv::erode       (in, out, erodeKernel, POINT_MINUS1, settings.erode_iterations  );           stats->erode        += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(in);            std::swap(in,out);                                                      
    cv::dilate      (in, out, dilateKernel,POINT_MINUS1, settings.dilate_iterations );           stats->dilate       += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(in);                               if (showWindows) { out.copyTo(img_eroded_dilated); } 
    cv::findContours(out, structures->all_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); stats->findContours += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(out);                                                                                    

    /* DEBUG purpose
    if (showWindows)
    {
        cv::imshow("inRange",       img_inRange );
        cv::imshow("GaussianBlur",  img_GaussianBlur );
        cv::imshow("eroded_dilated",img_eroded_dilated );
        cv::waitKey(1);
    }*/
}

void calc_center_of_contour(const std::vector<cv::Point2i>& points, cv::Point* center)
{
    cv::Moments M = cv::moments(points);
    center->x = int(M.m10 / M.m00);
	center->y = int(M.m01 / M.m00);
}

void calc_centers_of_contours(Contoures* found, const int minimalContourArea)
{
    for ( int i=0; i < found->all_contours.size(); ++i )
    {
        const auto& contour = found->all_contours[i];
        if ( cv::contourArea(contour) > minimalContourArea )
        {
            //
            // calc center and add it to centers
            //
            cv::Point2i centerPoint;
            calc_center_of_contour(contour, &centerPoint);
            //found->centers.emplace_back( centerPoint.x, centerPoint.y );
            found->centers.emplace_back(i, centerPoint.x, centerPoint.y);
            //
            // remeber index of found center to access the corresponding contour afterwards
            //
            //found->centers_contours_idx.push_back(i);
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

        if ( x_ref1 <= plant_x_abs_offset_from_0 && plant_x_abs_offset_from_0 < x_ref2 )
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

bool calc_overall_threshold_draw_plants(const ReflinesSettings& refSettings, const int frame_rows, Contoures* contoures, float* avg_threshold)
{
    const float threshold_percent = (float)refSettings.rowThresholdPx / (float)refSettings.rowSpacingPx;

    float   sum_threshold = 0;
    cv::Point plant_coord;

    uint centers_processed = 0;
    for ( int i=0; i < contoures->centers.size(); ++i )
    {
        Center& plant = contoures->centers[i];
        // hier bitte mit Magie befüllen!

        plant_coord.x = plant.point.x - refSettings.x_half;
        plant_coord.y = frame_rows - plant.point.y;

        float nearest_refLine_x;
        float deltaPx;
        float refLines_distance_px;
        if ( find_point_on_nearest_refline(plant_coord, refSettings, &nearest_refLine_x, &deltaPx, &refLines_distance_px) )
        {
            const float threshold = deltaPx / refLines_distance_px;
            sum_threshold += threshold;
            
            plant.within_threshold = std::abs(threshold) < threshold_percent;
            //const cv::Scalar& plant_color = std::abs(threshold) < threshold_percent ? BLUE : RED;
            //cv::drawMarker  ( frame, plant , plant_color, cv::MarkerTypes::MARKER_CROSS, 20, 2 );
            //cv::drawContours( frame, structures->all_contours, structures->centers_contours_idx[i], plant_color, 1 );
            centers_processed += 1;
        }
    }
    if ( centers_processed > 0 ) {
        *avg_threshold = sum_threshold / (float)centers_processed;
    }

    return centers_processed > 0;
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

void detect_main(Workitem* work, DetectContext* ctx)
{
    work->detect_result.reset();

    if ( ! work->isPictureFromCamera )
    {
        work->detect_result.state = DETECT_STATE::NO_VALID_FRAME;
        return;
    }

    if ( ctx->shared->harrowLifted.load() )
    {
        work->detect_result.state = DETECT_STATE::HARROW_LIFTED;
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();

    find_contours(
            ctx->shared->detectSettings.getImageSettings()
        ,   ctx->stats
        ,  work->frame               
        ,&(work->detect_result.contoures)
        ,   ctx->showDebugWindows );                                                           

    auto refline_settings = ctx->shared->detectSettings.getReflineSettings();

    calc_centers_of_contours(
        &(work->detect_result.contoures)
        , ctx->shared->detectSettings.getImageSettings().minimalContourArea);

    if ( work->detect_result.contoures.centers.size() == 0 )
    {
        work->detect_result.state = DETECT_STATE::NOTHING_FOUND;
    }
    else if ( ! calc_overall_threshold_draw_plants(
                    ctx->shared->detectSettings.getReflineSettings()
                    , work->frame.rows
                    , &(work->detect_result.contoures)
                    , &(work->detect_result.avg_threshold) ) )
    {
        work->detect_result.state = DETECT_STATE::NO_PLANTS_WITHIN_LINES;
    }
    else
    {
        work->detect_result.state           = DETECT_STATE::SUCCESS;
        work->detect_result.is_in_threshold = is_within_threshold(work->detect_result.avg_threshold, refline_settings.rowSpacingPx, refline_settings.rowThresholdPx);
        
        if ( ctx->harrow != nullptr )
        {
            HARROW_DIRECTION direction = get_harrow_direction(work->detect_result.is_in_threshold, work->detect_result.avg_threshold);
            ctx->harrow->move(direction, "detect");
        }
    }

    ctx->stats->overall += trk::get_duration(&start);
    ctx->stats->frames++;
}