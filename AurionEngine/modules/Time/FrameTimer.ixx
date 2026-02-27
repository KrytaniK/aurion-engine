module;

export module Aurion.Time:FrameTimer;

import Aurion.Types;
import :FrameTime;
import :Clock;

export namespace Aurion
{
    struct FrameTimerConfig
    {
        f64 fixedDeltaTime = 1.0 / 60.0;    // Default fixed time-step to 60Hz/16ms
        f64 maxDeltaTime = 5.0 / 60;        // Clamp deltaTime to avoid spiral of death
        u32 maxFixedSteps = 8;              // Hard cap on fixed steps per frame
    };

    class FrameTimer
    {
    public:
        FrameTimer(const FrameTimerConfig& config);
        ~FrameTimer();

        const FrameTime& BeginFrame();
        void EndFrame();

        bool BeginFixedStep();
        void EndFixedStep();

    private:
        Clock m_clock;
        FrameTimerConfig m_config;
        FrameTime m_frame;
        f64 m_accumulator;
        u32 m_steps_this_frame;
        Clock::Timepoint m_frame_end;
        Clock::Timepoint m_fixed_frame_end;
    };
}
