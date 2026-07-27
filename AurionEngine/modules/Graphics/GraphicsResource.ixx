module;

#include <vulkan/vulkan_raii.hpp>
#include <string>

export module Aurion.Graphics:GraphicsResource;

import Aurion.Resources;
import Aurion.Types;

export namespace Aurion
{
    class IGraphicsDriver;
    class GraphicsResource : public Resource
    {
    public:
        enum Type { None = 0, Buffer, Texture, Sampler, RenderTarget, Shader, Pipeline };

        struct Config
        {
            Config() = default;
            explicit Config(const Type& type) : rType(type) {};
            virtual ~Config() = default;

            [[nodiscard]] Type GetType() const { return rType; }

            std::string name{};

        private:
            Type rType = None;
        };

    public:
        explicit GraphicsResource(const std::string_view& id) : Resource(id) {};
        ~GraphicsResource() override = default;

        virtual void Configure(const Config* properties) = 0;

        virtual void Attach(const IGraphicsDriver* driver) = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;
    };
}