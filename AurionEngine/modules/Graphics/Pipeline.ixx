module;

#include <string>

export module Aurion.Graphics:Pipeline;

import :GraphicsResource;

export namespace Aurion
{
    class Pipeline : public GraphicsResource
    {
    public:
        enum Type { None = 0, Graphics, Compute, RayTrace };

        struct Config : GraphicsResource::Config
        {
            explicit Config(const Type& type) : GraphicsResource::Config(GraphicsResource::Pipeline), pType(type) {};

            Type pType = Graphics;
        };

    public:
        explicit Pipeline(const std::string_view& id) : GraphicsResource(id) {};
        ~Pipeline() override = default;

        void Configure(const GraphicsResource::Config* properties) override = 0;
        void Attach(const IGraphicsDriver* driver) override = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;
    };
}