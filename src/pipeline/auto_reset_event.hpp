#pragma once

#include <mutex>
#include <condition_variable>

class AutoResetEvent
{
  public:
    explicit AutoResetEvent(bool initial = false);

    void Set();
    void WaitOne();

  private:
    AutoResetEvent(const AutoResetEvent&);
    AutoResetEvent& operator=(const AutoResetEvent&); // non-copyable

    bool                    m_flag;
    std::mutex              m_protect;
    std::condition_variable m_signal;
};

