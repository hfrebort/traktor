#include <stdio.h>
#include <thread>
#include <chrono>

#include "stats.h"
#include "shared.h"

void thread_stats(Shared* shared, Stats* stats)
{
    for (;;)
    {
        const unsigned long fps = stats->fps.exchange(0);
        auto prepare_ms = fps > 0 ? ( (float)stats->prepare.exchange(0) / (float)fps / 1000000.0 ) : -1;

        printf("fps (%ld) camera_frames/s (%lu) prepare/ms (%5.1f)\n"
            , fps
            , stats->camera_frames.exchange(0)
            , prepare_ms);
            
        std::this_thread::sleep_for( std::chrono::milliseconds(2000) );

        if ( shared->shutdown_requested.load() )
        {
            break;
        }
    }
}