module;

#include <chrono>
#include <AurionLog.h>

module Aurion.Time;

import Aurion.Types;

namespace Aurion
{
    FrameTimer::FrameTimer(const FrameTimerConfig& config)
        : m_config(config), m_frame_start(Clock::SteadyClock::now()), m_fixed_frame_start(Clock::SteadyClock::now()),
          m_steps_this_frame(0)
    {
        m_frame.totalTime = 0.0f;
        m_frame.frameCount = 1;
    }

    FrameTimer::~FrameTimer()
    {
    }

    const FrameTime& FrameTimer::BeginFrame()
    {
        const Clock::Timepoint now = Clock::SteadyClock::now();

        // Track the time since the last frame; start new frame
        m_frame.deltaTime = Clock::Duration<f64, Clock::Seconds>(now - m_frame_start).count();
        m_frame_start = now;

        // Clamp the delta time to avoid spiral of death
        m_frame.deltaTime = m_frame.deltaTime > m_config.maxDeltaTime
                                ? m_config.maxDeltaTime
                                : m_frame.deltaTime;

        // Increase frame number and accumulate frame time
        m_frame.frameCount++;
        m_accumulator += m_frame.deltaTime;

        // Update the time since timer start
        m_frame.totalTime += m_frame.deltaTime;

        return m_frame;
    }

    bool FrameTimer::BeginFixedStep()
    {
        // Only consume a fixed step if we've accumulated frame debt
        //  and haven't over-stepped the catch-up boundary
        if (m_accumulator >= m_config.fixedDeltaTime &&
            m_steps_this_frame < m_config.maxFixedSteps)
        {
            const Clock::Timepoint now = Clock::SteadyClock::now();

            // Track the time since the last fixed frame; start new fixed frame
            m_frame.fixedDeltaTime = Clock::Duration<f64, Clock::Seconds>(now - m_fixed_frame_start).count();
            m_fixed_frame_start = now;

            m_accumulator -= m_config.fixedDeltaTime;

            m_frame.fixedStepCount++;
            m_steps_this_frame++;
            return true;
        }

        // The alpha value tells us how far between the last and next fixed update we are.
        m_frame.alpha = m_frame.fixedDeltaTime > 0.f
                            ? m_accumulator / m_frame.fixedDeltaTime
                            : 0.f;

        m_steps_this_frame = 0;
        return false;
    }
}
