#include <stdio.h>
#include <thread>
#include <chrono>

#include "stats.h"

void thread_stats(Stats* stats)
{

    for (;;)
    {
        printf("fps (%ld) camera_read (%lu) camera_frames (%lu)\n"
            , stats->fps.load()
            , stats->camera_read_ns.load()
            , stats->camera_frames.exchange(0));
            
        std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
    }
}