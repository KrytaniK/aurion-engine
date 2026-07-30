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
        class RenderPass
        {
        public:
            struct ResourceRef
            {
                std::string name{};
                u32 version = 0;

                bool operator==(const ResourceRef&) const = default;
            };

        public:
            // Basic description of a render pass. Includes the minimum required
            // members for use within the RenderGraph
            struct Config
            {
                std::string name; // The name of this render pass
                std::vector<ResourceRef> inputs; // The names of all resource dependencies
                std::vector<ResourceRef> outputs; // The names of the output resources
            };
        };
    }

    template <>
    struct std::hash<Aurion::RenderPass::ResourceRef>
    {
        std::size_t operator()(const Aurion::RenderPass::ResourceRef& r) const noexcept
        {
            std::size_t h = std::hash<std::string>{}(r.name);
            // Mix the version in; don't just XOR the two raw hashes.
            h ^= std::hash<Aurion::u32>{}(r.version) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            return h;
        }
    };
}