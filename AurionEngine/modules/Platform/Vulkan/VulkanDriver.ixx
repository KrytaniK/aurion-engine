module;

#include <vulkan/vulkan_raii.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <span>

export module Aurion.Vulkan:Driver;

import Aurion.Graphics;
import Aurion.Assets;
import Aurion.Types;
import Aurion.Utility;

import :Types;
import :Config;

import :Queue;

export namespace Aurion::Vulkan
{
    class API;

    class Driver : public IGraphicsDriver
    {
    public:
        explicit Driver(const API* vulkan_api, const PhysicalDeviceProperties& pDevice_props, const DeviceProperties& device_props);
        ~Driver() override;

        [[nodiscard]] SurfaceHandle CreateSurface(const SurfaceDescription& desc) override;


        // ---------- OLD IMPLEMENTATION ----------

        // --- Single Resource Allocation ---

        [[nodiscard]] PipelineHandle CreatePipeline(const PipelineDescription& desc) override;
        [[nodiscard]] ShaderHandle CreateShader(const ShaderDescription& desc) override;
        [[nodiscard]] BufferHandle CreateBuffer(const BufferDescription& desc) override;
        [[nodiscard]] TextureHandle CreateTexture(const TextureDescription& desc) override;
        [[nodiscard]] TextureViewHandle CreateTextureView(const TextureViewDescription& desc) override;
        [[nodiscard]] SamplerHandle CreateSampler(const SamplerDescription& desc) override;
        [[nodiscard]] RenderTargetHandle CreateRenderTarget(const RenderTargetDescription& desc) override;

        [[nodiscard]] ResourcePoolHandle CreateResourcePool(const ResourcePoolDescription& desc) override;
        [[nodiscard]] ResourceGroupLayoutHandle CreateResourceGroupLayout(const ResourceGroupLayoutDescription& desc) override;
        [[nodiscard]] ResourceGroupHandle AllocateResourceGroup(const ResourcePoolHandle& pool, const ResourceGroupLayoutHandle& layout) override;

        // --- Batch Resource Allocation ---

        [[nodiscard]] std::vector<PipelineHandle> CreatePipelines(std::span<PipelineDescription> descriptions) override;
        [[nodiscard]] std::vector<ShaderHandle> CreateShaders(std::span<ShaderDescription> descriptions) override;
        [[nodiscard]] std::vector<BufferHandle> CreateBuffers(std::span<BufferDescription> descriptions) override;
        [[nodiscard]] std::vector<TextureHandle> CreateTextures(std::span<TextureDescription> descriptions) override;
        [[nodiscard]] std::vector<TextureViewHandle> CreateTextureViews(std::span<TextureViewDescription> descriptions) override;
        [[nodiscard]] std::vector<SamplerHandle> CreateSamplers(std::span<SamplerDescription> descriptions) override;
        [[nodiscard]] std::vector<RenderTargetHandle> CreateRenderTargets(std::span<RenderTargetDescription> descriptions) override;

        [[nodiscard]] std::vector<ResourcePoolHandle> CreateResourcePools(std::span<ResourcePoolDescription> descriptions) override;
        [[nodiscard]] std::vector<ResourceGroupLayoutHandle> CreateResourceGroupLayouts(std::span<ResourceGroupLayoutDescription> descriptions) override;

        [[nodiscard]] std::vector<ResourceGroupHandle> AllocateResourceGroups(const ResourcePoolHandle& pool, const ResourceGroupLayoutHandle& layout, const u32& count) override;

        // --- Resource Cleanup ---

        void Release(GPUHandle& handle) override;

        // --- Resource & Memory Binding ---

        void UpdateResourceGroupBinding(const ResourceBindingUpdate& update) override;
        void UpdateResourceGroupBindings(std::span<ResourceBindingUpdate> updates) override;

        [[nodiscard]] ResourceMemoryRequirements QueryMemoryRequirements(const BufferDescription& buffer_desc) override;
        [[nodiscard]] ResourceMemoryRequirements QueryMemoryRequirements(const RenderTargetDescription& render_target_desc) override;

        [[nodiscard]] ResourceMemoryHandle AllocateResourceMemory(const ResourceMemoryRequirements& requirements) override;
        void BindResourceMemory(const GPUHandle& resource, const ResourceMemoryHandle& memory, const u64& offset) override;

        // --- Resource Handle Retrieval ---

        [[nodiscard]] ResourceGroupLayoutHandle GetShaderResourceGroupLayout(const ShaderHandle& shader) override;

    private:

        [[nodiscard]] SwapchainHandle CreateSwapchain(const SurfaceHandle& surface, const TextureDescription& image_desc, const TextureViewDescription& view_desc);

        // ---------- OLD IMPLEMENTATION ----------


        [[nodiscard]] ResourceGroupLayoutHandle RegisterResourceGroupLayout(vk::raii::DescriptorSetLayout layout, const ResourceGroupLayoutDescription& desc);

        [[nodiscard]] vk::raii::ShaderModule CompileShaderModule(
            const std::string& path,
            const std::vector<char>& code,
            const ShaderLanguage& lang,
            const ShaderEntryPoint& entry_point,
            const std::vector<ShaderMacro>& defines
        ) const;

        [[nodiscard]] std::shared_ptr<vk::raii::DeviceMemory> AllocateDeviceMemory(
            const vk::MemoryRequirements& mem_reqs,
            const vk::MemoryPropertyFlags& prop_flags
        ) const;

        // TODO: Window surface creation might need to be specific to the windowing API the engine uses.
        //      It's fine for now, since GLFW is the only option, but it might be worthwhile to make this
        //      flexible in the future.

        [[nodiscard]] vk::raii::SurfaceKHR CreateWindowSurface(Window* window) const;

        void ValidatePresentSupport(const vk::raii::SurfaceKHR& surface) const;

    private:
        const API* m_vulkan_api;
        vk::raii::PhysicalDevice m_physical_device;

        vk::raii::Device m_logical_device;
        std::unordered_map<u32, QueueFamily> m_queue_families;

        // --- Resource Pools ---

        std::vector<BufferData> m_buffers;
        std::vector<TextureData> m_textures;
        std::vector<TextureViewData> m_texture_views;
        std::vector<SamplerData> m_samplers;
        std::vector<RenderTargetData> m_render_targets;

        std::vector<ResourceMemoryData> m_resource_memory;

        // --- Persistent Data ---

        std::vector<ShaderData> m_shaders;
        std::vector<PipelineData> m_pipelines;
        std::vector<SurfaceData> m_surfaces;
        std::vector<SwapchainData> m_swapchains;

        // --- Per-Shader Binding Data ---

        std::vector<ResourcePoolData> m_resource_pools;
        std::vector<ResourceGroupLayoutData> m_resource_group_layouts;
        std::vector<ResourceGroupData> m_resource_groups;
    };
}
