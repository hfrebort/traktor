#include <atomic>
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
