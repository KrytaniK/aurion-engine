module;

#include <string>

export module Aurion.Graphics:GraphicsResource;

import Aurion.Resources;
import Aurion.Types;

export namespace Aurion
{
    enum GPUResourceType
    {
        AURION_GPU_RESOURCE_NULL = 0,
        AURION_GPU_RESOURCE_TEXTURE,
        AURION_GPU_RESOURCE_SAMPLER,
        AURION_GPU_RESOURCE_BUFFER,
    };

    struct GPUResourceConfig
    {
        GPUResourceType rType = AURION_GPU_RESOURCE_NULL;
    };

    struct GPUResourceDescription
    {
        std::string name; // Name of the resource (MUST be unique!)
        u64 id; // Unique ID, hashed from resource name "aurion-engine" links to:

        GPUResourceConfig* config;
    };
}