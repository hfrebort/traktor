#pragma once

//#include <condition_variable>
//#include <thread>
//#include <atomic>
//#include <opencv2/core.hpp>
//
//#include "httplib.h"

#include "stats.h"
#include "util.h"

struct Options
{
    bool        showDebugWindows;
    int         cameraIndex;
    int         httpPort;
    std::string filename;
    int         video_playback_slowdown_factor;
    int         camera_width;
    int         camera_fps;

    Options() :
          showDebugWindows(false)
        , cameraIndex(0)
        , httpPort(9080)
        , video_playback_slowdown_factor(1)
        , camera_width(640)
        , camera_fps(25)
    {}
};

struct ImageSettings {
    public:
        int         frame_cols;
        int         frame_rows;
        cv::Scalar  colorFrom;
        cv::Scalar  colorTo;
        int         maxPlats;
        //int         maxZentimeter;
        int         erode_iterations;
        int         dilate_iterations;
        int         minimalContourArea;

    ImageSettings()
    {
        colorFrom          = cv::Scalar(36,  25,  25);
        colorTo            = cv::Scalar(86, 255, 255);
        erode_iterations   = 3;
        dilate_iterations  = 3;
        maxPlats           = 10;
        minimalContourArea = 130;
    }
};

struct ReflinesSettings {
    public:
        int         rowCount;
        int         rowSpacingPx;
        int         rowPerspectivePx;
        int         rowThresholdPx;
        
        int         x_half;
        int         y_fluchtpunkt;
        int         half_row_count;

    ReflinesSettings()
    {
        rowCount           = 3;
        half_row_count     = 1;
        rowSpacingPx       = 160;
        rowPerspectivePx   = 0;
        rowThresholdPx     = 1;

        y_fluchtpunkt      = 0;
    }
};

struct DetectSettings {
    private:
        ImageSettings       imageSettings;
        ReflinesSettings    reflineSettings;

        void recalculate_rowCount()
        {
            //const int x_half        = set.frame_cols / 2;
            //const int fluchtpunkt_y = set.frame_rows + set.rowPerspectivePx;
            const int x_max_outer_row = ( imageSettings.frame_rows / reflineSettings.x_half ) * reflineSettings.y_fluchtpunkt;

            const int rows_calculated_on_one_side = x_max_outer_row / reflineSettings.rowSpacingPx;
            reflineSettings.rowCount = rows_calculated_on_one_side * 2 + 1;

            reflineSettings.half_row_count  = (reflineSettings.rowCount-1) / 2;
        }

    public:

        const ImageSettings&    getImageSettings()   { return imageSettings;   }
        const ReflinesSettings& getReflineSettings() { return reflineSettings; }

        void set_frame(const int newCols, const int newRows) {  
            imageSettings.frame_cols = newCols;
            imageSettings.frame_rows = newRows;

            reflineSettings.x_half = newCols / 2;
            reflineSettings.y_fluchtpunkt = reflineSettings.rowPerspectivePx + imageSettings.frame_rows;

            recalculate_rowCount();
        }

        void set_rowPerspectivePx(const int perspectivePx) {  
            reflineSettings.rowPerspectivePx = perspectivePx;
            reflineSettings.y_fluchtpunkt = reflineSettings.rowPerspectivePx + imageSettings.frame_rows;
            recalculate_rowCount();
        }

        void set_colorFrom(const std::string& csvValues) {
            trk::setColorFromCSV( csvValues, imageSettings.colorFrom);
        }

        void set_colorTo(const std::string& csvValues) {
            trk::setColorFromCSV( csvValues, imageSettings.colorTo);
        }

        void set_rowSpacingPx(const int newRowSpacingPx) {
            reflineSettings.rowSpacingPx = newRowSpacingPx;
            recalculate_rowCount();
        }

        void set_rowThresholdPx(const int newRowThresholdPx) {
            reflineSettings.rowThresholdPx = newRowThresholdPx;
        }
};

struct Shared {
    public:

        Stats                   stats;
        httplib::Server         *webSvr;

        std::atomic<bool>       shutdown_requested;
        //
        // sync between camera an detection thread
        //
        cv::Mat                 frame_buf[3];
        std::atomic<int>        frame_buf_slot;
        std::condition_variable camera_frame_ready;
        std::mutex              camera_frame_ready_mutex;
        //
        // sync between detection and sending thread
        //
        cv::Mat                 analyzed_frame_buf[2];
        std::mutex              analyzed_frame_buf_mutex[2];
        std::condition_variable analyzed_frame_ready;
        std::mutex              analyzed_frame_ready_mutex;
        std::atomic<int>        analyzed_frame_ready_idx;
        std::atomic<bool>       analyzed_frame_encoded_to_JPEG;
        std::vector<uchar>      analyzed_frame_jpegImage;

        DetectSettings          detectSettings;

    Shared()
    {
        shutdown_requested.store(false);
        webSvr = nullptr;
        frame_buf_slot.store(-1);
        analyzed_frame_ready_idx.store(-1);
        analyzed_frame_encoded_to_JPEG.store(false);
    }
};