#include "harrow.h"

void move_harrow_towards_center(Harrow* harrow)
{
    if ( harrow->isZweitLinks() == 0 )
    {
        harrow->move(HARROW_DIRECTION::RIGHT, "center");
    }
    else if ( harrow->isZweitRechts() == 0 )
    {
        harrow->move(HARROW_DIRECTION::LEFT, "center");
    }
    else 
    {
        harrow->move(HARROW_DIRECTION::STOP, "center");
    }
}

void thread_center_harrow(Harrow* harrow, std::atomic<bool>* harrowLifted, const std::atomic<bool>* shutdown_requested)
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
            perror("E: could not read sensor LIFTED");
            break;
        }

        if ( lifted == 1 )
        {
            harrowLifted->store(false);
        }
        else if ( lifted == 0 )
        {
            harrowLifted->store(true);
            move_harrow_towards_center(harrow);
        }

        if ( shutdown_requested->load() )
        {
            break;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(50) );
    }
}