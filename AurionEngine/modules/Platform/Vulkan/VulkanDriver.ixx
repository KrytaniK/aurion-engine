module;

#include <vulkan/vulkan_raii.hpp>
#include <unordered_map>
#include <vector>

export module Aurion.Vulkan:Driver;

import Aurion.Graphics;
import Aurion.Resources;
import Aurion.Types;

import :Config;
import :Queue;
import :RenderPass;

import :RenderTarget;
import :Buffer;
import :Pipeline;

export namespace Aurion::Vulkan
{
    class Driver : public IGraphicsDriver
    {
    public:
        explicit Driver(const DriverConfig& config);
        ~Driver() override;

        void BeginFrame() override;
        void RecordCommands() override;
        void EndFrame() override;

        // Pipeline Generation From RenderGraph Output
        void ResolveFrameGraph(const FrameGraph& graph) override;

        // Resource Creation
        // -----------------------------------------------------

        // Creates a blank buffer, tied to this driver.
        ResourceHandle<Aurion::Buffer> CreateBuffer(const std::string_view& id) override;

        // Creates a blank render target, tied to this driver.
        ResourceHandle<Aurion::RenderTarget> CreateRenderTarget(const std::string_view& id) override;

        // Creates a blank shader, tied to this driver.
        ResourceHandle<Shader> CreateShader(const std::string_view& id) override;

        // Creates a blank pipeline, tied to this driver.
        ResourceHandle<Pipeline> CreatePipeline(const std::string_view& id, const Pipeline::Type& type) override;

        // Resource Creation Utility Functions
        // -----------------------------------------------------

        // Ensures the provided surface can be presented to
        void ValidatePresentSupport(const vk::raii::SurfaceKHR& surface) const;

        [[nodiscard]] vk::raii::SurfaceKHR CreateWindowSurface(Window* window) const;

        [[nodiscard]] vk::raii::SwapchainKHR CreateSwapchain(
            const vk::raii::SurfaceKHR& surface,
            const RenderTarget::Config& properties,
            vk::raii::SwapchainKHR* old_swapchain = nullptr
        ) const;

        [[nodiscard]] std::vector<vk::raii::ImageView> CreateImageViews(
            const std::span<vk::Image>& images,
            const RenderTarget::Config& properties
        ) const;

        [[nodiscard]] vk::raii::Buffer AllocateBuffer(const Vulkan::Buffer::Config& config) const;
        [[nodiscard]] vk::raii::DeviceMemory AllocateBufferMemory(const vk::raii::Buffer& buffer, vk::MemoryPropertyFlags prop_flags) const;

        [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(
            const std::string_view& path,
            const std::vector<char>& code,
            const Shader::Language& lang,
            const Shader::EntryPoint& entry_point,
            const std::vector<Shader::Macro>& defines
        ) const;

        [[nodiscard]] vk::raii::PipelineLayout BuildPipelineLayout(const vk::PipelineLayoutCreateInfo& info) const;
        [[nodiscard]] vk::raii::Pipeline BuildGraphicsPipeline(const Vulkan::GraphicsPipeline::Config& config) const;

    private:
        // A helper function to create the application resources a render pass depends on
        [[nodiscard]] RenderPass::Resources ResolveRenderPassResources(const std::vector<GraphicsResource::Config*>& configs);
        //[[nodiscard]] vk::raii::CommandBuffer& ResolveRenderPassCommandBuffer(const RenderPassOp& op, const u32& channel, const u32& index);

        // Vulkan Pipeline Helpers
        [[nodiscard]] vk::ShaderStageFlagBits GetVulkanPipelineStage(const Aurion::Shader::Stage& stage) const;

    private:
        ResourceManager* m_resource_manager;
        const vk::raii::Context* m_context;
        const vk::raii::Instance* m_instance;
        vk::raii::PhysicalDevice m_physical_device;
        vk::raii::Device m_logical_device;
        std::unordered_map<u32, QueueFamily> m_queue_families;
        SurfaceCreateFn m_CreateSurfaceFn;
        std::unordered_map<u64, vk::raii::SurfaceKHR> m_surfaces;
    };
}
