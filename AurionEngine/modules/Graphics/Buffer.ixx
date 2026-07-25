module;



export module Aurion.Graphics:Buffer;

import Aurion.Resources;
import Aurion.Types;

import :GraphicsResource;

export namespace Aurion
{
    class Buffer : public GraphicsResource
    {
    public:
        struct Config : GraphicsResource::Config
        {
            Config() : GraphicsResource::Config(GraphicsResource::Buffer) {}

            u32 size = 0;
        };

    public:
        explicit Buffer(const std::string_view& id) : GraphicsResource(id) {};
        ~Buffer() override = default;

        void Configure(const GraphicsResource::Config* properties) override = 0;

        void Attach(const IGraphicsDriver* driver) override = 0;

        virtual void Write(void* data, const u32& offset, const u32& size) = 0;

        [[nodiscard]] virtual u32 GetSize() const = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;
    };
}