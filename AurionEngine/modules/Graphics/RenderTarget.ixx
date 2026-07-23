module;

#include <string>

export module Aurion.Graphics:RenderTarget;

import Aurion.Resources;
import Aurion.Window;
import Aurion.Types;

import :GraphicsResource;

export namespace Aurion
{
    class RenderTarget : public Resource
    {
    public:
        struct Config : GraphicsResourceConfig
        {
            Config() : GraphicsResourceConfig(AURION_GPU_RESOURCE_RENDER_TARGET) {}

            u32 width = 0;
            u32 height = 0;
            u32 frames_in_flight = 3;
            bool vSync_enabled = true;
        };

    public:
        explicit RenderTarget(const std::string_view& id) : Resource(id) {};
        ~RenderTarget() override = default;

        virtual void Configure(const Config* properties) = 0;

        virtual void Attach(Window* window) = 0;

        virtual void Validate() = 0;

        [[nodiscard]] virtual u32 GetWidth() const = 0;
        [[nodiscard]] virtual u32 GetHeight() const = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;
    };
}