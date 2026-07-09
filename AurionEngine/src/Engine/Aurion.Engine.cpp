module;

#include "AurionLog.h"
#include <vector>
#include <chrono>

module Aurion.Engine;

import Aurion.Events;
import Aurion.Time;
import Aurion.Types;

namespace Aurion
{
    void Engine::Initialize(int argc, char* argv[])
    {
        AURION_INFO("Initializing Engine...");
    }

    void Engine::OnEvent(EventBase* event)
    {
        AURION_INFO("Engine On Event!");
    }

    void Engine::Run()
    {
        FrameTimer timer({
            .fixedDeltaTime     = 1.0 / 60.0,   // ~16.67ms per fixed frame (Could be refresh rate of monitor)
            .maxDeltaTime       = 1.0 / 60.0,   // ~16.67ms max frame time
            .maxFixedSteps      = 8             // Max catchup of 8 fixed steps
        });

        auto start = Clock::SteadyClock::now();

        m_shouldClose = false;
        while (!m_shouldClose)
        {
            const FrameTime& ft = timer.BeginFrame();

            if (ft.totalTime >= 1.0)
            {
                m_shouldClose = true;
                continue;
            }

            AURION_INFO("Frame: %d | Actual FPS: %.2f", ft.frameCount, ft.frameCount / ft.totalTime);

            while (timer.BeginFixedStep())
            {
                // Fixed Update: Deterministic Logic (such as physics)
            }

            // Normal Update: Gameplay, Input, Animation, Etc

            // Late Update: Dependent Updates

            // Rendering
        }
    }

    void Engine::Shutdown()
    {

    }
}
