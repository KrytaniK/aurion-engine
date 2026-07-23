module;

#include <vector>
#include <span>
#include <memory>

export module Aurion.Graphics:RenderGraph;

import Aurion.Types;

import :GraphicsResource;
import :RenderPass;

export namespace Aurion
{
    struct FrameGraph
    {
        // The order in which render passes should be executed
        std::vector<u64> execution_order;

        // The list of render pass descriptions
        std::vector<std::shared_ptr<RenderPassDescription>> pass_descriptions;

        // A collection of GPU resource descriptions. Resolved by backend rendering API
        std::vector<std::shared_ptr<GraphicsResourceConfig>> resource_descriptions;
    };

    class RenderGraph
    {
    public:
        explicit RenderGraph() = default;
        ~RenderGraph();

        // Attaches an API-specific resource configuration structure
        //  to this render graph.
        template<typename Resource>
        void AddResource(const typename Resource::Config& desc);

        // Attaches an API-specific render pass description structure
        //  to this render graph
        template<typename Pass>
        void AddPass(const typename Pass::Config& desc);

        // Compile render passes into an execution graph
        [[nodiscard]] FrameGraph Compile();

    private:
        // Executes a topological sort of all render passes, based
        // on render pass dependencies and outputs
        [[nodiscard]] std::vector<u64> TopologicalSort(const std::span<std::vector<u64>>& dependency_matrix) const;

    private:
        std::vector<std::shared_ptr<GraphicsResourceConfig>> m_resource_descriptions{};
        std::vector<std::shared_ptr<RenderPassDescription>> m_pass_descriptions{};
    };

    template<typename Resource>
    void RenderGraph::AddResource(const typename Resource::Config& desc)
    {
        static_assert(std::is_base_of_v<GraphicsResourceConfig, typename Resource::Config>, "Resource Config struct must derive from Aurion::GraphicsResourceConfig");
        m_resource_descriptions.push_back(std::make_shared<typename Resource::Config>(desc));
    }

    template <typename Pass>
    void RenderGraph::AddPass(const typename Pass::Config& desc)
    {
        static_assert(std::is_base_of_v<RenderPassDescription, typename Pass::Config>, "Resource Config struct must derive from Aurion::RenderPassDescription");
        m_pass_descriptions.push_back(std::make_shared<typename Pass::Config>(desc));
    }
}
