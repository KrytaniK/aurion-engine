module;

#include <string>

export module Aurion.Graphics:RenderTarget;

import Aurion.Resources;
import Aurion.Window;
import Aurion.Types;

import :GraphicsResource;
import :Texture;

export namespace Aurion
{
    class RenderTarget : public GraphicsResource
    {
    public:
        struct Config : GraphicsResource::Config
        {
            Config() : GraphicsResource::Config(GraphicsResource::RenderTarget) {}

            u32 frames_in_flight = 1;
        };

    public:
        explicit RenderTarget(const std::string_view& id) : GraphicsResource(id) {};
        ~RenderTarget() override = default;

        void Configure(const GraphicsResource::Config* properties) override = 0;

        void Attach(const IGraphicsDriver* driver) override = 0;

        // Swap internal image buffers for this target, and return the new index
        virtual u32 SwapBuffers(const u32& frame_index) = 0;

        [[nodiscard]] virtual u32 GetWidth() const = 0;
        [[nodiscard]] virtual u32 GetHeight() const = 0;
        [[nodiscard]] virtual u32 GetDepth() const = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;
    };
}