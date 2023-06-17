#pragma once

#include <atomic>

#include "auto_reset_event.hpp"

class Postbox
{

public:

    Postbox(int32_t init_value);

    int32_t exchange(const int32_t val);
    bool    compare_exchange(int32_t& expected, int32_t desired);
    void    signal();
    void    wait_for_signal();

private:

    std::atomic_int32_t     _box;
    AutoResetEvent          _signal;

};