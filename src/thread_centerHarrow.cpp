#include "harrow.h"

void move_harrow_towards_center(Harrow* harrow)
{
    if ( harrow->isZweitLinks() )
    {
        harrow->move(HARROW_DIRECTION::RIGHT);
    }
    else if ( harrow->isZweitRechts() )
    {
        harrow->move(HARROW_DIRECTION::LEFT);
    }
    else 
    {
        harrow->move(HARROW_DIRECTION::STOP);
    }
}

void thread_centerHarrow(Harrow* harrow, std::atomic<bool>* shouldMoveHarrow, const std::atomic<bool>* shutdown_requested)
{
    if ( harrow == nullptr )
    {
        perror("E: thread_center_harrow: harrow == nullptr. exiting.");
        return;
    }

    puts("I: thread center harrow running");

    for (;;)
    {
        const int lifted = harrow->isLifted();
        if      ( lifted == -1 )
        {
            perror("could not read value from sensor LIFTED");
            break;
        }

        if ( lifted == 0 )
        {
            shouldMoveHarrow->store(true);
        }
        else if ( lifted == 1 )
        {
            shouldMoveHarrow->store(false);
            move_harrow_towards_center(harrow);
        }

        if ( shutdown_requested->load() )
        {
            break;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(100) );
    }
}