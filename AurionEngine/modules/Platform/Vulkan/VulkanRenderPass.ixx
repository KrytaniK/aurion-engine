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

    struct RenderPass : Aurion::RenderPass
    {
        RenderPassExecFn OnExecute = nullptr; // The execution function for this render pass
    };
}