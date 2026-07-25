module;

#include <vulkan/vulkan_raii.hpp>
#include <string>

module Aurion.Vulkan;

import Aurion.Graphics;
import Aurion.Types;

namespace Aurion::Vulkan
{
    GraphicsPipeline::GraphicsPipeline(const std::string_view& id)
        : Pipeline(id)
    {

    }

    GraphicsPipeline::~GraphicsPipeline()
    {

    }

    void GraphicsPipeline::Configure(const GraphicsResource::Config* properties)
    {
        
    }

    bool GraphicsPipeline::OnLoad()
    {
        return true;
    }

    bool GraphicsPipeline::OnUnload()
    {
        return true;
    }
}
