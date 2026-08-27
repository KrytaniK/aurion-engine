module;

#include <vulkan/vulkan_raii.hpp>

export module Aurion.Vulkan:RenderContext;

import Aurion.Graphics;

import :API;
import :Types;
import :CommandList;

export namespace Aurion::Vulkan
{
    class Driver;
    class RenderContext : public IRenderContext
    {
    public:
        explicit RenderContext(
            Driver* driver,
            const vk::raii::Device& logical_device,
            const CommandBufferGroup& buffers,
            const u32& max_frame_count
        );
        ~RenderContext() override;

        void BeginFrame() override;
        void Draw(const RenderGraphCompilationResult& graph) override;
        void EndFrame() override;

        [[nodiscard]] const u32& GetFrameIndex() const override;
        [[nodiscard]] const Extent& GetRenderExtent() const override;

    private:
        Driver* m_driver;
        const vk::raii::Device* m_logical_device;
        std::vector<vk::raii::Fence> m_fences;
        std::vector<CommandList> m_graphics_buffers;
        std::vector<CommandList> m_compute_buffers;
        std::vector<CommandList> m_transfer_buffers;
        FrameContext m_frame_context;
        RenderGraphResource m_present_target;
    };
}