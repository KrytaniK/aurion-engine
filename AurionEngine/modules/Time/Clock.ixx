module;

#include <chrono>

export module Aurion.Time:Clock;

import Aurion.Types;

export namespace Aurion
{
    class Clock
    {
    public:
        // Aliasing for std::chrono
        using SteadyClock = std::chrono::steady_clock;
        using Timepoint = SteadyClock::time_point;

        template<typename Rep, typename Period=  std::ratio<1>>
        using Duration = std::chrono::duration<Rep, Period>;
        using Nanoseconds   = std::ratio<1, 1000000000>;
        using Microseconds  = std::ratio<1, 1000000>;
        using MilliSeconds  = std::ratio<1, 1000>;
        using Seconds       = std::ratio<1, 1>;

        static f64 TimeSinceStart();

        Clock();
        virtual ~Clock() = default;

        void Reset();

        template<typename Rep, typename Period=  std::ratio<1>>
        [[nodiscard]] Duration<Rep, Period> ElapsedTime() const;
    private:
        Timepoint m_start;
    };
}