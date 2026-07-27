module;

#include <vulkan/vulkan_raii.hpp>
#include <functional>
#include <vector>

export module Aurion.Vulkan:RenderPass;

import Aurion.Graphics;
import Aurion.Resources;

import :RenderTarget;

export namespace Aurion::Vulkan
{
    struct FrameContext;
    typedef std::function<void(const FrameContext&)> RenderPassExecFn;

    class RenderPass
    {
    public:
        struct Config : Aurion::RenderPass::Config
        {
            // Pipeline Information
            std::vector<std::string> pipelines;

            // Queue/Command Buffer Selection
            const u32 queue_index = 0; // The preferred queue for rendering.
            const u32 command_buffer_index = 0; // The index of the preferred command buffer

            RenderPassExecFn OnExecute = nullptr; // The execution function for this render pass
        };

        struct Resources
        {
            std::vector<ResourceHandle<Buffer>> buffers;
            // std::vector<ResourceHandle<bool>> textures;
            // std::vector<ResourceHandle<bool>> samplers;
            std::vector<ResourceHandle<RenderTarget>> render_targets;
            // std::vector<ResourceHandle<bool>> graphics_pipelines;
            // std::vector<ResourceHandle<bool>> compute_pipelines;
            // std::vector<ResourceHandle<bool>> raytrace_pipelines;
        };

    public:

    private:

    };

    struct FrameContext
    {
        const vk::raii::CommandBuffer& cmd_buffer;
        RenderPass::Resources inputs;
        RenderPass::Resources outputs;
    };
}