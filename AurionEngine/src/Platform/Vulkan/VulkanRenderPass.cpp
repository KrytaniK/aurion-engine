module;

#include <vulkan/vulkan_raii.hpp>

module Aurion.Vulkan;

namespace Aurion::Vulkan
{
    RenderPass::RenderPass(const Config& config)
    {

    }

    void RenderPass::Execute(const vk::raii::CommandBuffer& cmd)
    {
        // Build Frame Context
        // Execute function
    }
}
