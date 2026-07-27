module;

#include <vector>
#include <string>

export module Aurion.Graphics:RenderPass;

import :GraphicsResource;
import :RenderTarget;
import :Shader;

export namespace Aurion
{
    class RenderPass
    {
    public:
        // Basic description of a render pass. Includes the minimum required
        // members for use within the RenderGraph
        struct Config
        {
            std::string name; // The name of this render pass
            std::vector<std::string> inputs; // The names of all resource dependencies
            std::vector<std::string> outputs; // The names of the output resources
        };
    };
}