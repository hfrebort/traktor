#pragma once

#include "stats.h"

class Detect
{
    public:

        Detect(Stats& stats) : _stats(stats) {}
        void run();

    private:

        Stats& _stats;

};