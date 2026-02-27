#include <chrono>

import Aurion.Time;
import Aurion.Types;

namespace Aurion
{
    f64 Clock::TimeSinceStart()
    {
        static Timepoint s_app_start = SteadyClock::now();
        return Clock::Duration<f64, Seconds>(SteadyClock::now() - s_app_start).count();
    }

    Clock::Clock()
        : m_start(SteadyClock::now())
    {

    }

    void Clock::Reset()
    {
        m_start = SteadyClock::now();
    }

    template<typename Rep, typename Period>
    Clock::Duration<Rep, Period> Clock::ElapsedTime() const
    {
        return Clock::Duration<Rep, Period>(m_start - SteadyClock::now());
    }
}
