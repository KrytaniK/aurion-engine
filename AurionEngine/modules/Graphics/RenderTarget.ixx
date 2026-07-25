module;

#include <string>

export module Aurion.Graphics:RenderTarget;

import Aurion.Resources;
import Aurion.Window;
import Aurion.Types;

import :GraphicsResource;

export namespace Aurion
{
    class RenderTarget : public GraphicsResource
    {
    public:
        struct Config : GraphicsResource::Config
        {
            Config() : GraphicsResource::Config(GraphicsResource::RenderTarget) {}

            Window* window = nullptr;
            u32 width = 0;
            u32 height = 0;
            u32 frames_in_flight = 3;
            bool vSync_enabled = true;
        };

    public:
        explicit RenderTarget(const std::string_view& id) : GraphicsResource(id) {};
        ~RenderTarget() override = default;

        void Configure(const GraphicsResource::Config* properties) override = 0;

        void Attach(const IGraphicsDriver* driver) override = 0;

        virtual void Validate() = 0;

        [[nodiscard]] virtual u32 GetWidth() const = 0;
        [[nodiscard]] virtual u32 GetHeight() const = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;
    };
}