module;

#include <vulkan/vulkan_raii.hpp>

module Aurion.Vulkan;

namespace Aurion::Vulkan
{
    QueueFamily::QueueFamily()
    {
    }

    void QueueFamily::GenerateCommandPool(const vk::raii::Device& device, const vk::CommandPoolCreateFlagBits& create_flags)
    {
        vk::CommandPoolCreateInfo createInfo = {};
        createInfo.flags = create_flags;
        createInfo.queueFamilyIndex = this->index;

        m_cmd_pool = std::move(vk::raii::CommandPool(device, createInfo));
    }

    std::span<vk::raii::CommandBuffer> QueueFamily::AllocateCommandBuffers(const vk::raii::Device& device, const vk::CommandBufferLevel& buffer_level, const u32& count)
    {
        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.commandPool = m_cmd_pool;
        allocInfo.level = buffer_level;
        allocInfo.commandBufferCount = count;

        std::vector<vk::raii::CommandBuffer>& batch = m_cmd_buffer_batches.emplace_back(vk::raii::CommandBuffers(device, allocInfo));
        return std::span<vk::raii::CommandBuffer>(batch);
    }

    const QueueData& QueueFamily::GetQueue(const vk::raii::Device& device, const u32& index)
    {
        const auto it = m_queues.find(index);
        if (it != m_queues.end())
            return it->second;

        QueueData data{};
        data.family_index = this->index;
        data.handle = vk::raii::Queue(device, this->index, index);

        const auto [inserted_it, success] = m_queues.emplace(index, std::move(data));
        return inserted_it->second;
    }
}
