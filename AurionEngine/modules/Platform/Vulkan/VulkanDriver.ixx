module;

#include <vulkan/vulkan_raii.hpp>
#include <unordered_map>
#include <string>

export module Aurion.Vulkan:Driver;

import Aurion.Graphics;
import Aurion.Resources;
import Aurion.Types;

import :RenderTarget;
import :Config;
import :Queue;
import :RenderPass;

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

        // Creates a blank render target, tied to the logical device
        ResourceHandle<Aurion::RenderTarget> CreateRenderTarget(const std::string_view& id) override;

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

    private:
        // A helper function to create the application resources a render pass depends on
        [[nodiscard]] RenderPass::Resources ResolveRenderPassResources(const std::vector<GraphicsResourceConfig*>& configs);
        [[nodiscard]] vk::raii::CommandBuffer& ResolveRenderPassCommandBuffer(const RenderPassOp& op, const u32& channel, const u32& index);

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
