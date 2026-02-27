module;

export module Aurion.Time:FrameTime;

import Aurion.Types;

export namespace Aurion
{
    // POD struct for per-frame snapshot statistics
    struct FrameTime
    {
        f64 deltaTime = 0.0;          // Variable delta time for this frame (in seconds)
        f64 totalTime = 0.0;          // Total time since engine start (in seconds)
        f64 fixedDeltaTime = 0.0;     // Fixed timestep interval (in seconds)
        f64 alpha = 0.0;              // Frame Interpolation factor for rendering: [0, 1
        u64 frameCount = 0;           // Total elapsed frames
        u64 fixedStepCount = 0;       // Total elapsed fixed timesteps
    };
}
