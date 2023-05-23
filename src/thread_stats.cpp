//#include <stdio.h>
//#include <thread>
//#include <chrono>

#include "stats.h"
#include "shared.h"

float ns_to_ms_per_fps(uint64 ns, int fps)
{
    return fps > 0 ? 
                  ( ( (float)ns / (float)fps ) / 1000000.0 ) 
                : -1;
}

void thread_stats(const std::atomic_bool& shutdown_requested, const Stats& stats, Stats* diff)
{
    Stats last;

    for (;;)
    {
        std::this_thread::sleep_for( stats.pause );

        Stats::diff(stats, last, diff);
        last = stats;

        if ( shutdown_requested.load() )
        {
            break;
        }
    }
}


/*
void thread_stats(Shared* shared, Stats* stats)
{
    const int secondsToPause = 2;

    for (;;)
    {
        int fps = stats->fps.exchange(0);
        if ( fps != 0 )
        {
            printf("fps (%2d) camera_fps (%2d) DETECT (%4.1f) { prepare (%4.1f) contours (%3.1f) calc_draw (%3.1f) } JPEG kB/s (%lu) image processing MB/s (%lu)\n"
                   "  prepare { cvtColor %4.1f GaussianBlur %4.1f inRange %4.1f erode %4.1f dilate %4.1f } copyTo (%4.6f) copyTo: %lu MB/s\n" 
                , fps / secondsToPause
                , stats->camera_frames.exchange(0) / secondsToPause
                , ns_to_ms_per_fps(stats->detect_overall_ns      .exchange(0), fps)
                , ns_to_ms_per_fps(stats->prepare_ns             .exchange(0), fps)
                , ns_to_ms_per_fps(stats->findContours_ns        .exchange(0), fps)
                , ns_to_ms_per_fps(stats->calc_draw_ns           .exchange(0), fps)
                ,                 (stats->jpeg_bytes_sent        .exchange(0) / 1024 /        secondsToPause)
                ,                 (stats->frame_bytes_processed  .exchange(0) / 1024 / 1024 / secondsToPause)

                , ns_to_ms_per_fps(stats->prepare_cvtColor_ns    .exchange(0), fps)      
                , ns_to_ms_per_fps(stats->prepare_GaussianBlur_ns.exchange(0), fps)      
                , ns_to_ms_per_fps(stats->prepare_inRange_ns     .exchange(0), fps)      
                , ns_to_ms_per_fps(stats->prepare_erode_ns       .exchange(0), fps)      
                , ns_to_ms_per_fps(stats->prepare_dilate_ns      .exchange(0), fps)      
                , ns_to_ms_per_fps(stats->copyTo_ns              .exchange(0), fps)
                ,                 (stats->copyTo_bytes           .exchange(0) / 1024 / 1024 / secondsToPause)

                );
        }

        std::this_thread::sleep_for( std::chrono::seconds(secondsToPause) );

        if ( shared->shutdown_requested.load() )
        {
            break;
        }
    }
}
*/