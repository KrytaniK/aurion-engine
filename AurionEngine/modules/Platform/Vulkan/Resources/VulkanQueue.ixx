module;

#include <vulkan/vulkan_raii.hpp>
#include <vector>

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

        // Allocates command buffers for each queue in the queue family, with a scaling factor to handle
        //  in-flight frames.
        void AllocateCommandBuffers(const vk::raii::Device& device, const vk::CommandBufferLevel& buffer_level, const u32& scale = 1);

        [[nodiscard]] QueueData GetQueue(const vk::raii::Device& device, const u32& index);

        [[nodiscard]] const vk::raii::CommandBuffer& GetCommandBuffer(const u32& index);

    public:
        u32 index = 0;

    private:
        u32 m_scale;
        vk::raii::CommandPool m_cmd_pool = nullptr;
        std::vector<vk::raii::CommandBuffer> m_cmd_buffers = {};
    };
}