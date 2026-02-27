#include <chrono>
#include <AurionLog.h>

import Aurion.Time;
import Aurion.Types;

namespace Aurion
{
    FrameTimer::FrameTimer(const FrameTimerConfig& config)
        : m_config(config), m_frame_end(Clock::SteadyClock::now())
    {
        AURION_WARN("Frame Count: %u", m_frame.frameCount);
    }

    FrameTimer::~FrameTimer()
    {

    }

    const FrameTime& FrameTimer::BeginFrame()
    {
        // Track the time since the last frame
        m_frame.deltaTime = Clock::Duration<f64, Clock::MilliSeconds>(Clock::SteadyClock::now() - m_frame_end).count();

        // Clamp the delta time to avoid spiral of death
        m_frame.deltaTime = m_frame.deltaTime > m_config.fixedDeltaTime
            ? m_config.maxDeltaTime
            : m_frame.deltaTime;

        // Increase frame number and accumulate frame time
        m_frame.frameCount++;
        m_accumulator += m_frame.deltaTime;

        // Track the time since engine start
        m_frame.totalTime = Clock::TimeSinceStart();

        return m_frame;
    }

    void FrameTimer::EndFrame()
    {
        m_frame_end = Clock::SteadyClock::now();
    }

    bool FrameTimer::BeginFixedStep()
    {
        // Only consume a fixed step if we've accumulated frame debt
        //  and haven't over-stepped the catch-up boundary
        if (m_accumulator >= m_config.fixedDeltaTime &&
            m_steps_this_frame < m_config.maxFixedSteps)
        {
            // Track the time since the last fixed frame
            m_frame.fixedDeltaTime =
                Clock::Duration<f64, Clock::MilliSeconds>(Clock::SteadyClock::now() - m_fixed_frame_end).count();

            m_accumulator -= m_config.fixedDeltaTime;
            m_frame.fixedStepCount++;
            m_steps_this_frame++;
            return true;
        }

        // The alpha value tells us how far between the last and next fixed update we are.
        m_frame.alpha = m_accumulator / m_config.fixedDeltaTime;

        m_steps_this_frame = 0;
        return false;
    }

    void FrameTimer::EndFixedStep()
    {
        m_fixed_frame_end = Clock::SteadyClock::now();
    }
}
