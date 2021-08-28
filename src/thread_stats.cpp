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

void thread_stats(Shared* shared, Stats* stats)
{
    const int secondsToPause = 2;

    for (;;)
    {
        int fps = stats->fps.exchange(0);
        if ( fps != 0 )
        {
            printf("fps (%2d) camera_fps (%2d) detect (%4.1f) { prepare (%4.1f) contours (%3.1f) calc (%3.1f) } draw (%3.1f) JPEG kB/s (%lu)\n"
                , fps / secondsToPause
                , stats->camera_frames.exchange(0) / secondsToPause
                , ns_to_ms_per_fps(stats->detect_overall_ns   .exchange(0), fps)
                , ns_to_ms_per_fps(stats->prepare_ns          .exchange(0), fps)
                , ns_to_ms_per_fps(stats->findContours_ns     .exchange(0), fps)
                , ns_to_ms_per_fps(stats->calcCenters_ns      .exchange(0), fps)
                , ns_to_ms_per_fps(stats->draw_ns             .exchange(0), fps)
                ,                 (stats->jpeg_bytes_sent     .exchange(0) / 1024 / secondsToPause)
                );
        }

        std::this_thread::sleep_for( std::chrono::seconds(secondsToPause) );

        if ( shared->shutdown_requested.load() )
        {
            break;
        }
    }
}