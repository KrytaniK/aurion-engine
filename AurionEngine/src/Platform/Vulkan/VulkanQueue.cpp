module;

#include <vulkan/vulkan_raii.hpp>

module Aurion.Vulkan;

namespace Aurion::Vulkan
{
    QueueFamily::QueueFamily()
        : m_scale(1)
    {
    }

    void QueueFamily::GenerateCommandPool(const vk::raii::Device& device, const vk::CommandPoolCreateFlagBits& create_flags)
    {
        vk::CommandPoolCreateInfo createInfo = {};
        createInfo.flags = create_flags;
        createInfo.queueFamilyIndex = this->index;

        m_cmd_pool = std::move(vk::raii::CommandPool(device, createInfo));
    }

    void QueueFamily::AllocateCommandBuffers(const vk::raii::Device& device, const vk::CommandBufferLevel& buffer_level, const u32& scale)
    {
        m_scale = scale;

        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.commandPool = m_cmd_pool;
        allocInfo.level = buffer_level;
        allocInfo.commandBufferCount = queueCount * scale;

        m_cmd_buffers = std::move(vk::raii::CommandBuffers(device, allocInfo));
    }

    Queue QueueFamily::GetQueue(const vk::raii::Device& device, const u32& index)
    {
        const u64 start = std::min(static_cast<u64>(index * m_scale), m_cmd_buffers.size());
        const u64 end = std::min(start + m_scale, m_cmd_buffers.size());

        return {
            .family_index = this->index,
            .handle = vk::raii::Queue(device, this->index, index),
            .command_buffers = {m_cmd_buffers.begin() + start, m_cmd_buffers.begin() + end},
        };
    }

    const vk::raii::CommandBuffer& QueueFamily::GetCommandBuffer(const u32& index)
    {
        return m_cmd_buffers[index % m_cmd_buffers.size()];
    }
}
