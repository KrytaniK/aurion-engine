import Aurion.Engine;
import Aurion.Events;
import Aurion.Time;
import Aurion.Types;

#include "AurionLog.h"

namespace Aurion
{
    void Engine::Initialize(int argc, char* argv[])
    {

    }

    void Engine::OnEvent(EventBase* event)
    {

    }

    void Engine::Run()
    {
        FrameTimer timer({
            .fixedDeltaTime     = 1000 * 1.0 / 60.0,   // ~16.67ms per fixed frame (Could be refresh rate of monitor)
            .maxDeltaTime       = 1000 * 5.0 / 60.0,   // ~83.3ms max frame time
            .maxFixedSteps      = 8             // Max catchup of 8 fixed steps
        });

        m_shouldClose = false;
        while (!m_shouldClose)
        {
            const FrameTime& ft = timer.BeginFrame();

            while (timer.BeginFixedStep())
            {
                // Fixed Update: Deterministic Logic (such as physics)

                timer.EndFixedStep();
            }

            // Normal Update: Gameplay, Input, Animation, Etc

            // Late Update: Dependent Updates

            // Rendering

            timer.EndFrame();
        }
    }

    void Engine::Shutdown()
    {

    }
}
