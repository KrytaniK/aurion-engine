module;

#include <vulkan/vulkan_raii.hpp>

module Aurion.Vulkan;

namespace Aurion::Vulkan
{
    void QueueFamily::GenerateCommandPool(const vk::raii::Device& device, const vk::CommandPoolCreateFlagBits& create_flags)
    {
        vk::CommandPoolCreateInfo createInfo = {};
        createInfo.flags = create_flags;
        createInfo.queueFamilyIndex = this->index;

        m_cmd_pool = std::move(vk::raii::CommandPool(device, createInfo));
    }

    void QueueFamily::AllocateCommandBuffers(const vk::raii::Device& device, const vk::CommandBufferLevel& buffer_level, const u32& scale)
    {
        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.commandPool = m_cmd_pool;
        allocInfo.level = buffer_level;
        allocInfo.commandBufferCount = queueCount * scale;

        m_cmd_buffers = std::move(vk::raii::CommandBuffers(device, allocInfo));
    }
}