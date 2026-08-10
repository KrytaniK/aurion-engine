module;

#include <vulkan/vulkan_raii.hpp>
#include <functional>
#include <vector>

export module Aurion.Vulkan:RenderPass;

import Aurion.Graphics;
import Aurion.Resources;

import :Buffer;
import :RenderTarget;
import :Pipeline;

export namespace Aurion::Vulkan
{
    typedef std::function<void(const vk::raii::CommandBuffer&)> RenderPassExecFn;

    class RenderPass
    {
    public:
        struct Config : Aurion::RenderPass::Config
        {
            // Pipeline names
            std::vector<std::string> pipelines;

            RenderPassExecFn OnExecute = nullptr; // The execution function for this render pass
        };

    public:
        explicit RenderPass(const Config& config);
        ~RenderPass() = default;

        void Execute(const vk::raii::CommandBuffer& cmd);

    private:
        RenderPassExecFn m_OnExecute;
        std::vector<GraphicsResource*> inputs;
        std::vector<GraphicsResource*> outputs;
        std::vector<ResourceHandle<Pipeline>> pipelines;
    };
}