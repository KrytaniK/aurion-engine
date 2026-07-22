module;

#include <string>

export module Aurion.Graphics:RenderTarget;

import Aurion.Resources;
import Aurion.Window;
import Aurion.Types;

export namespace Aurion
{
    struct RenderTargetProperties
    {
        u32 width = 0;
        u32 height = 0;
        u32 frames_in_flight = 3; // Default to triple buffering
        bool vSync_enabled = true; // Default enable vsync
    };

    class RenderTarget : public Resource
    {
    public:
        explicit RenderTarget(const std::string_view& id) : Resource(id) {};
        ~RenderTarget() override = default;

        virtual void Configure(const RenderTargetProperties* properties) = 0;

        virtual void Attach(Window* window) = 0;

        virtual void Validate() = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;
    };
}