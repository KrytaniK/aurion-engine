module;

#include <vulkan/vulkan_raii.hpp>
#include <string>
#include <vector>

module Aurion.Vulkan;

import Aurion.Resources;
import Aurion.Graphics;
import Aurion.Types;

namespace Aurion::Vulkan
{
    GraphicsPipeline::GraphicsPipeline(const std::string_view& id)
        : Pipeline(id), m_driver(nullptr), m_resource_manager(nullptr)
    {

    }

    GraphicsPipeline::~GraphicsPipeline()
    {

    }

    void GraphicsPipeline::Configure(const GraphicsResource::Config* properties)
    {
        m_config = *dynamic_cast<const Config*>(properties);

        // Build and assign layout
        m_layout = m_driver->BuildGraphicsPipelineLayout(m_config);
        m_config.layout = m_layout;

        // Build graphics pipeline
        m_pipeline = m_driver->BuildGraphicsPipeline(m_config);

        // null config stages, since they're shader-dependent
        m_config.stageCount = 0;
        m_config.pStages = nullptr;
    }

    void GraphicsPipeline::Attach(const IGraphicsDriver* driver)
    {
        m_driver = dynamic_cast<const Driver*>(driver);
    }

    bool GraphicsPipeline::OnLoad()
    {
        m_resource_manager = ServiceLocator::GetService<ResourceManager>();
        return m_resource_manager != nullptr;
    }

    bool GraphicsPipeline::OnUnload()
    {
        m_resource_manager = nullptr;
        return true;
    }
}
