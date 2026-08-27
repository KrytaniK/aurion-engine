module;

#include <vulkan/vulkan_raii.hpp>
#include <unordered_map>
#include <memory>

export module Aurion.Vulkan:Types;

import Aurion.Graphics;
import Aurion.Types;
import Aurion.Utility;

export namespace Aurion::Vulkan
{
    // Vulkan-Specific Handles

    struct SwapchainHandle : GPUHandle {};

    struct QueueData
    {
        u32 family_index = 0;
        vk::raii::Queue handle = nullptr;
    };

    struct BufferData
    {
        vk::raii::Buffer buffer = nullptr;
        std::shared_ptr<vk::raii::DeviceMemory> memory{}; // null until bound (transient path binds later)
        void* mapped_memory = nullptr;
        BufferDescription desc{};
    };

    struct TextureData
    {
        vk::raii::Image image = nullptr;
        std::shared_ptr<vk::raii::DeviceMemory> memory{};
        TextureDescription desc{};
    };

    struct TextureViewData
    {
        vk::raii::ImageView view = nullptr;
        TextureViewDescription desc{};
        vk::Filter nearest;
        vk::SamplerMipmapMode mmmode;
        vk::SamplerAddressMode addrMode;
        vk::BorderColor bc;
        vk::CompareOp cmp_op;
    };

    struct SamplerData
    {
        vk::raii::Sampler sampler = nullptr;
        SamplerDescription desc{};
    };

    struct ShaderData
    {
        std::unordered_map<ShaderStage, vk::raii::ShaderModule> modules{};
        std::vector<ShaderEntryPoint> entry_points{};
        ResourceGroupLayoutHandle group_layout{}; // Descriptor set layout derived from resource_bindings
    };

    struct PipelineData
    {
        vk::raii::Pipeline pipeline = nullptr;
        vk::raii::PipelineLayout layout = nullptr;
    };

    struct SurfaceData
    {
        vk::raii::SurfaceKHR surface = nullptr;
        SurfaceDescription desc{};
    };

    struct SwapchainData
    {
        SurfaceHandle surface{};
        vk::raii::SwapchainKHR swapchain = nullptr;
        Extent extent{};
        std::vector<vk::Image> images{};
        std::vector<vk::raii::ImageView> views{};
        std::vector<vk::raii::Semaphore> acquire_semaphores{};
        std::vector<vk::raii::Semaphore> present_semaphores{};
        u32 image_index = 0;
    };

    struct RenderTargetData
    {
        SwapchainHandle swapchain{}; // For swapchain-backed render targets
        TextureHandle image{}; // Image handle for 'offline' render targets
        TextureViewHandle view{}; // View handle for the 'offline' image
        RenderTargetDescription desc{};
    };

    struct ResourcePoolData
    {
        vk::raii::DescriptorPool pool = nullptr;
        ResourcePoolDescription desc{};
    };

    struct ResourceGroupLayoutData
    {
        vk::raii::DescriptorSetLayout layout = nullptr;
        ResourceGroupLayoutDescription desc{};
    };

    struct ResourceGroupData
    {
        vk::raii::DescriptorSet set = nullptr;
        ResourceGroupLayoutHandle layout{}; // Layout this group was allocated from - resolves binding types
    };

    struct ResourceMemoryData
    {
        std::shared_ptr<vk::raii::DeviceMemory> memory{};
        u64 size = 0;
    };

    struct CommandBufferGroup
    {
        std::pair<const QueueData*, std::span<vk::raii::CommandBuffer>> graphics{};
        std::pair<const QueueData*, std::span<vk::raii::CommandBuffer>> compute{};
        std::pair<const QueueData*, std::span<vk::raii::CommandBuffer>> transfer{};
    };
}
