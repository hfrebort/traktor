#include "auto_reset_event.hpp"

AutoResetEvent::AutoResetEvent(bool initial)
: m_flag(initial)
{
}

void AutoResetEvent::Set()
{
    {
        std::lock_guard<std::mutex> guard(m_protect);
        m_flag = true;
        // guard is destructed here. lock is released
    }
    m_signal.notify_one();
}

void AutoResetEvent::WaitOne()
{
    {
        std::unique_lock<std::mutex> lk(m_protect);
        while( !m_flag ) // prevent spurious wakeups from doing harm
        {
            m_signal.wait(lk);
        }
        m_flag = false; // waiting resets the flag
        // lk is destructed here. lock is released
    }
}