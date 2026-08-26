module;

#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include <unordered_map>
#include <span>

export module Aurion.Vulkan:Queue;

import Aurion.Graphics;
import Aurion.Types;

import :Types;
import :Config;

export namespace Aurion::Vulkan
{
    // A wrapper around VkQueueFamilyProperties to encapsulate associated command buffers
    class QueueFamily : public vk::QueueFamilyProperties
    {
    public:
        explicit QueueFamily();

        // Generates a command pool using the specified creation flags
        void GenerateCommandPool(const vk::raii::Device& device, const vk::CommandPoolCreateFlagBits& create_flags);

        // Allocates `count` new command buffers from this family's pool, on top of any previously
        //  allocated ones, and returns a span over just the newly allocated buffers.
        [[nodiscard]] std::span<vk::raii::CommandBuffer> AllocateCommandBuffers(const vk::raii::Device& device, const vk::CommandBufferLevel& buffer_level, const u32& count);

        // Returns the queue at the given index within this family, creating (and caching) it on
        //  first access. Repeated calls with the same index return the same cached QueueData.
        [[nodiscard]] const QueueData& GetQueue(const vk::raii::Device& device, const u32& index);

    public:
        u32 index = 0;

    private:
        vk::raii::CommandPool m_cmd_pool = nullptr;
        std::unordered_map<u32, QueueData> m_queues{};
        std::vector<std::vector<vk::raii::CommandBuffer>> m_cmd_buffer_batches{};
    };
}