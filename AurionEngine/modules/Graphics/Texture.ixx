module;

#include <string>

export module Aurion.Graphics:Texture;

import :GraphicsResource;

export namespace Aurion
{
    class Texture : public GraphicsResource
    {
    public:
        struct Config : GraphicsResource::Config
        {
            Config() : GraphicsResource::Config(GraphicsResource::Texture) {}

            u32 width = 0;
            u32 height = 0;
            u32 depth = 1;
        };

    public:
        explicit Texture(const std::string_view& id) : GraphicsResource(id) {};
        ~Texture() override = default;

        void Configure(const GraphicsResource::Config* properties) override = 0;
        void Attach(const IGraphicsDriver* driver) override = 0;

        [[nodiscard]] virtual u32 GetWidth() const = 0;
        [[nodiscard]] virtual u32 GetHeight() const = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;
    };
}