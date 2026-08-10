module;

#include <string>

export module Aurion.Graphics:RenderGraph;

import Aurion.Resources;
import Aurion.Types;

import :Buffer;
import :RenderTarget;
import :RenderPass;

export namespace Aurion
{
    class RenderGraph
    {
    public:
        explicit RenderGraph() = default;
        virtual ~RenderGraph() = default;

        // External, static resource referencing

        virtual void RegisterBuffer(const ResourceHandle<Buffer>& buffer) = 0;
        virtual void RegisterRenderTarget(const ResourceHandle<RenderTarget>& render_target) = 0;

        // Per-Frame, transient resource creation

        virtual const ResourceHandle<Buffer>& CreateBuffer(const Buffer::Config* desc) = 0;
        virtual const ResourceHandle<RenderTarget>& CreateRenderTarget(const RenderTarget::Config* desc) = 0;

        // Adds a render pass description (node) to this graph
        virtual void AddPass(const RenderPass::Config* desc) = 0;

        // Resolves resource dependencies and determines final pass execution order.
        virtual void Compile() = 0;

        // Binds this render graph to a transient/registered render target
        //  for presenting/copying
        virtual void Export(const std::string& render_target, const u32& version) = 0;

        virtual const ResourceHandle<RenderTarget>& GetExportTarget() const = 0;
    };
}
