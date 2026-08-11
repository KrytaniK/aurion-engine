module;

#include <vector>
#include <string>

export module Aurion.Graphics:RenderPass;

import Aurion.Types;

import :GraphicsResource;
import :RenderTarget;
import :Shader;

export {
    namespace Aurion
    {
        struct RenderPassResource
        {
            std::string name{};
            u32 version = 0;

            bool operator==(const RenderPassResource&) const = default;
        };

        // Basic description of a render pass. Includes the minimum required
        // members for use within the RenderGraph
        struct RenderPass
        {
            std::string name; // The name of this render pass
            std::vector<RenderPassResource> inputs; // The names of all resource dependencies
            std::vector<RenderPassResource> outputs; // The names of the output resources
        };
    }

    template <>
    struct std::hash<Aurion::RenderPassResource>
    {
        std::size_t operator()(const Aurion::RenderPassResource& r) const noexcept
        {
            std::size_t h = std::hash<std::string>{}(r.name);
            // Mix the version in; don't just XOR the two raw hashes.
            h ^= std::hash<Aurion::u32>{}(r.version) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            return h;
        }
    };
}