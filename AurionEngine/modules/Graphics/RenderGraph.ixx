module;

#include <vector>
#include <span>

export module Aurion.Graphics:RenderGraph;

import Aurion.Types;

import :GraphicsResource;
import :RenderPass;

export namespace Aurion
{
    struct RenderPipelineDescription
    {
        // The order in which render passes should be executed
        std::vector<u64> execution_order;

        // The list of render pass descriptions
        std::vector<RenderPassDescription> pass_descriptions;

        // A collection of GPU resource descriptions. Resolved by renderer
        std::vector<GPUResourceDescription> resource_descriptions;

        // TODO (FUTURE): Extend to handle parallel pass execution
    };

    class RenderGraph
    {
    public:
        explicit RenderGraph() = default;
        ~RenderGraph();

        // Add a graphics resource to this render pipeline
        void AddResource(const GPUResourceDescription& desc);

        // Add a render pass to this render pipeline
        void AddPass(const RenderPassDescription& pass);

        // Compile render passes into an execution graph
        RenderPipelineDescription Compile();

    private:
        // Executes a topological sort of all render passes, based
        // on render pass dependencies and outputs
        std::vector<u64> TopologicalSort(const std::span<std::vector<u64>>& dependency_matrix) const;

    private:
        std::vector<GPUResourceDescription> m_resource_descriptions{0};
        std::vector<RenderPassDescription> m_pass_descriptions{0};
    };
}