#include <stdio.h>
#include <thread>
#include <chrono>

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
    for (;;)
    {
        int fps = stats->fps.exchange(0);
        if ( fps != 0 )
        {
            printf("fps (%2d) camera_fps (%2lu) detect (%4.1f) { prepare (%3.1f) contours (%3.1f) calc (%3.1f) draw (%3.1f) }\n"
                , fps
                , stats->camera_frames.exchange(0)
                , ns_to_ms_per_fps(stats->detect_overall_ns   .exchange(0), fps)
                , ns_to_ms_per_fps(stats->prepare_ns          .exchange(0), fps)
                , ns_to_ms_per_fps(stats->findContours_ns     .exchange(0), fps)
                , ns_to_ms_per_fps(stats->calcCenters_ns      .exchange(0), fps)
                , ns_to_ms_per_fps(stats->draw_ns             .exchange(0), fps)
                );
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(2000) );

        if ( shared->shutdown_requested.load() )
        {
            break;
        }
    }
}