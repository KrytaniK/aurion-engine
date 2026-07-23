module;

#include <vulkan/vulkan_raii.hpp>
#include <functional>
#include <vector>

export module Aurion.Vulkan:RenderPass;

import Aurion.Graphics;

import :RenderTarget;

export namespace Aurion::Vulkan
{
    struct FrameContext;
    typedef std::function<void(const FrameContext&)> RenderPassExecFn;

    // TODO: Make This Class Useful!
    class RenderPass
    {
    public:
        struct Config : Aurion::RenderPassDescription
        {
            RenderPassExecFn OnExecute = nullptr;
        };

        struct Resources
        {
            std::vector<ResourceHandle<RenderTarget>> render_targets;
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