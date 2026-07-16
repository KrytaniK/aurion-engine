module;

#include <unordered_map>
#include <vector>
#include <span>
#include <functional>
#include <stdexcept>

module Aurion.Graphics;

namespace Aurion
{
    RenderGraph::~RenderGraph()
    {
        // Cleanup any remaining data
        m_pass_descriptions.clear();

        // If a graph was never compiled, clean up dangling pointers
        for (const auto& [_, __, config] : m_resource_descriptions)
            delete config;
    }

    void RenderGraph::AddResource(const GPUResourceDescription& desc)
    {
        m_resource_descriptions.push_back(desc);
    }

    void RenderGraph::AddPass(const RenderPassDescription& pass)
    {
        m_pass_descriptions.push_back(pass);
    }

    RenderPipelineDescription RenderGraph::Compile()
    {
        // First, build the dependency graph

        // Tracks the dependents (children) of each render pass, via index
        std::vector<std::vector<u64>> dep_matrix(m_pass_descriptions.size());

        // Track which passes produce which resource (resource name to pass index)
        std::unordered_map<std::string, u64> writes{};

        // Analyze each pass to determine resource relationships
        for (u64 i = 0; i < m_pass_descriptions.size(); ++i)
        {
            const auto& pass = m_pass_descriptions[i];

            // Process all inputs for this pass:
            // If a write to this resource by another pass was found,
            // assign the writing pass as a dependency of this pass.
            for (const auto& input : pass.inputs)
                if (const auto it = writes.find(input); it != writes.end())
                    dep_matrix[i].push_back(it->second);

            // For every pass, ensure the writes (outputs) are tracked.
            // Assuming each pass is in-order, later passes will overwrite this,
            // should another render pass write to the same resource
            for (const auto& output : pass.outputs)
                writes[output] = i;
        }

        RenderPipelineDescription rpd{
            .execution_order = TopologicalSort(dep_matrix), // Topologically sort passes to determine execution order
            .pass_descriptions = m_pass_descriptions, // Copy render pass descriptions
            .resource_descriptions = m_resource_descriptions, // Copy resource descriptions
        };

        // Clear internal state:
        // NOTE: Once compilation is complete, the RenderGraph no longer handles config pointers
        m_pass_descriptions.clear();
        m_resource_descriptions.clear();

        return rpd;
    }

    std::vector<u64> RenderGraph::TopologicalSort(const std::span<std::vector<u64>>& dependency_matrix) const
    {
        std::vector<u64> execution_order{};

        // Uses depth-first search to compute a valid execution sequence, without cycles.
        std::vector<bool> visited(m_pass_descriptions.size(), false);
        std::vector<bool> stack(m_pass_descriptions.size(), false);

        std::function < void(u64) > visit = [&](const u64& node)
        {
            // If the stack already contains this node, there is a
            // circular dependency
            if (stack[node])
                throw std::runtime_error("[RenderGraph] Render Pass Cycle Detected");

            // Don't process nodes twice
            if (visited[node])
                return;

            // Add this node to the stack, if it hasn't been processed.
            stack[node] = true;

            // Then, recursively process all dependents
            for (const auto& dep : dependency_matrix[node])
                visit(dep);

            // Once all dependents have been processed:
            stack[node] = false; // Remove from the call-stack
            visited[node] = true; // Mark this pass as processed
            execution_order.push_back(node); // Add index to execution sequence
        };

        // Process all unvisited nodes
        for (size_t i = 0; i < m_pass_descriptions.size(); ++i)
            if (!visited[i]) visit(i);

        // Return the final execution order
        return execution_order;
    }
}
