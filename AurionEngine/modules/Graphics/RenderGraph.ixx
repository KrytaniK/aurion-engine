module;

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <span>

export module Aurion.Graphics:RenderGraph;

import :Interface;
import :Types;
import :Config;

export namespace Aurion
{
    class RenderGraph
    {
    public:
        explicit RenderGraph(const std::shared_ptr<IGraphicsDriver>& driver);
        ~RenderGraph();

        // Registers a persistent resource with the graph
        [[nodiscard]] const VirtualHandle& ImportResource(const std::string_view& name, const GPUHandle& handle);

        // Queues the creation for a transient (per-frame) buffer
        [[nodiscard]] const VirtualHandle& CreateTransientBuffer(const std::string_view& name, const BufferDescription& desc);

        // Queues the creation for a transient (per-frame) render target
        [[nodiscard]] const VirtualHandle& CreateTransientRenderTarget(const std::string_view& name, const RenderTargetDescription& desc);

        void AddPass(const RenderPassDescription& desc);

        void Export(const std::string_view& resource_name, const u64& generation, const ResourceUsageIntent& output_usage);

        RenderGraphCompilationResult Compile();

        [[nodiscard]] const RenderGraphResource* GetExportTarget() const;

    private:
        [[nodiscard]] std::vector<std::vector<u64>> BuildDependencyGraph() const;

        [[nodiscard]] std::vector<u64> CullPasses(std::span<std::vector<u64>> graph) const;

        [[nodiscard]] std::vector<u64> SortPassesTopologically(std::span<std::vector<u64>> graph, std::span<u64> mask) const;

        [[nodiscard]] std::vector<std::pair<u64, u64>> GetResourceLifetimes(std::span<u64> execution_order) const;

        void AliasResources(std::span<std::pair<u64, u64>> lifetimes);

        void ResolvePassResourceHandles();

    private:
        std::shared_ptr<IGraphicsDriver> m_graphics_driver;
        RenderGraphResource m_export_target;
        std::vector<VirtualHandle> m_resources;
        std::vector<BufferDescription> m_buffer_descriptions;
        std::vector<RenderTargetDescription> m_render_target_descriptions;
        std::vector<RenderPassDescription> m_passes;
        std::vector<u64> m_execution_order;
    };

    // class RenderGraph
    // {
    // public:
    //     struct CompilationResult
    //     {
    //         ResourceHandle<RenderTarget> export_target;
    //         std::vector<std::shared_ptr<RenderPass>> passes;
    //         std::vector<u64> execution_order;
    //     };
    //
    // public:
    //     explicit RenderGraph() = default;
    //     virtual ~RenderGraph() = default;
    //
    //     // External, static resource referencing
    //
    //     virtual void RegisterBuffer(const ResourceHandle<Buffer>& buffer) = 0;
    //     virtual void RegisterRenderTarget(const ResourceHandle<RenderTarget>& render_target) = 0;
    //
    //     // Per-Frame, transient resource creation
    //
    //     virtual const ResourceHandle<Buffer>& CreateBuffer(const Buffer::Config* desc) = 0;
    //     virtual const ResourceHandle<RenderTarget>& CreateRenderTarget(const RenderTarget::Config* desc) = 0;
    //
    //     // Adds a render pass description (node) to this graph
    //     virtual void AddPass(const RenderPass* desc) = 0;
    //
    //     // Resolves resource dependencies and determines final pass execution order.
    //     [[nodiscard]] virtual CompilationResult Compile() = 0;
    //
    //     // Binds this render graph to a transient/registered render target
    //     //  for presenting/copying
    //     virtual void Export(const std::string& render_target, const u32& version) = 0;
    //
    //     [[nodiscard]] virtual const ResourceHandle<RenderTarget>& GetExportTarget() const = 0;
    // };
}
