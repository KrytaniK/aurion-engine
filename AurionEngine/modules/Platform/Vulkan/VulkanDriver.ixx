module;

#include <vulkan/vulkan_raii.hpp>
#include <unordered_map>
#include <vector>
#include <memory>

export module Aurion.Vulkan:Driver;

import Aurion.Graphics;
import Aurion.Resources;
import Aurion.Types;

import :Config;
import :Queue;
import :RenderPass;

import :Buffer;
import :Texture;
import :RenderTarget;
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

        // Resource Creation
        // -----------------------------------------------------

        // Creates a blank buffer, tied to this driver.
        ResourceHandle<Aurion::Buffer> CreateBuffer(const std::string_view& id) override;

        // Creates a blank texture, tied to this driver.
        ResourceHandle<Aurion::Texture> CreateTexture(const std::string_view& id) override;

        // Creates a blank render target, tied to this driver.
        ResourceHandle<Aurion::RenderTarget> CreateRenderTarget(const std::string_view& id) override;

        // Creates a blank shader, tied to this driver.
        ResourceHandle<Shader> CreateShader(const std::string_view& id) override;

        // Creates a blank pipeline, tied to this driver.
        ResourceHandle<Aurion::Pipeline> CreatePipeline(const std::string_view& id, const Pipeline::Type& type) override;

        // Synchronization Primitives

        [[nodiscard]] vk::raii::Semaphore CreateSemaphore(const vk::SemaphoreCreateInfo& info) const;
        [[nodiscard]] vk::raii::Fence CreateFence(const vk::FenceCreateInfo& info) const;

        // Resource Creation Utility Functions
        // -----------------------------------------------------

        // Allocation

        [[nodiscard]] std::shared_ptr<vk::raii::DeviceMemory> AllocateDeviceMemory(
            const vk::MemoryRequirements& mem_reqs,
            const vk::MemoryPropertyFlags& prop_flags
        ) const;

        [[nodiscard]] vk::raii::Buffer AllocateBuffer(const Vulkan::Buffer::Config& config) const;

        [[nodiscard]] vk::raii::Image AllocateImage(const Vulkan::Texture::Config& config) const;

        [[nodiscard]] vk::raii::ImageView AllocateImageView(const vk::Image& image, const vk::ImageViewCreateInfo& config) const;

        // Memory Binding

        [[nodiscard]] vk::raii::DeviceMemory AllocateBufferMemory(const vk::raii::Buffer& buffer, vk::MemoryPropertyFlags prop_flags) const;

        // Ensures the provided surface can be presented to
        void ValidatePresentSupport(const vk::raii::SurfaceKHR& surface) const;

        [[nodiscard]] vk::raii::SurfaceKHR CreateWindowSurface(Window* window) const;

        [[nodiscard]] vk::raii::SwapchainKHR CreateSwapchain(
            const vk::raii::SurfaceKHR& surface,
            const RenderTarget::SwapchainConfig& properties,
            vk::raii::SwapchainKHR* old_swapchain = nullptr
        ) const;

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
