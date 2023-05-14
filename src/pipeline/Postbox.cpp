#include "Postbox.hpp"

int32_t Postbox::exchange(const int32_t val)
{
    const int32_t value_in_the_box = _box.exchange(val, std::memory_order_release);

    return value_in_the_box;
}

bool Postbox::compare_exchange(int32_t& expected, int32_t desired)
{
    return 
        _box.compare_exchange_strong(
                expected,                   
                desired,                    
                std::memory_order_acq_rel,    // compare success: memory order for read-modify-write
                std::memory_order_acquire);   // compare failure: memory order for load
}

void Postbox::wait_for_signal()
{
    _signal.WaitOne();
}

void Postbox::signal()
{
    _signal.Set();
}

Postbox::Postbox(int32_t init_value)
{
    _box.store(init_value);
}
