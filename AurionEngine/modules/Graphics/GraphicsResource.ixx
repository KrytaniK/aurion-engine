module;

#include <string>

#include <vulkan/vulkan_raii.hpp>

export module Aurion.Graphics:GraphicsResource;

import Aurion.Resources;
import Aurion.Types;

export namespace Aurion
{
    enum GraphicsResourceType
    {
        AURION_GPU_RESOURCE_NULL = 0,
        AURION_GPU_RESOURCE_RENDER_TARGET,
        AURION_GPU_RESOURCE_TEXTURE,
        AURION_GPU_RESOURCE_SAMPLER,
        AURION_GPU_RESOURCE_BUFFER,
    };

    struct GraphicsResourceConfig
    {
        GraphicsResourceConfig() = default;
        explicit GraphicsResourceConfig(const GraphicsResourceType& type) : rType(type) {};
        virtual ~GraphicsResourceConfig() = default;

        std::string name{};
        GraphicsResourceType rType = AURION_GPU_RESOURCE_NULL;
    };
}