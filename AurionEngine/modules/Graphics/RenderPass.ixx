module;

#include <vector>
#include <string>

export module Aurion.Graphics:RenderPass;

import :GraphicsResource;

export namespace Aurion
{
    // Base POD for describing a render pass. Includes the minimum required
    // members for use within the RenderGraph
    struct RenderPassDescription
    {
        std::string name;
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
    };
}