module;

#include <AurionLog.h>
#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include <ranges>
#include <span>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>
#include <stdexcept>

module Aurion.Vulkan;

import Aurion.Services;
import Aurion.Types;

namespace Aurion::Vulkan
{
    RenderGraph::RenderGraph(const std::shared_ptr<IGraphicsDriver>& driver)
        : Aurion::RenderGraph()
    {
        m_driver = std::dynamic_pointer_cast<Vulkan::Driver>(driver);
    }

    void RenderGraph::RegisterBuffer(const ResourceHandle<Aurion::Buffer>& buffer)
    {
        m_buffers.push_back(buffer);
    }

    void RenderGraph::RegisterRenderTarget(const ResourceHandle<Aurion::RenderTarget>& render_target)
    {
        m_render_targets.push_back(render_target);
    }

    const ResourceHandle<Aurion::Buffer>& RenderGraph::CreateBuffer(const Aurion::Buffer::Config* desc)
    {
        // Cache the config structure for later processing
        m_buffer_configs.emplace_back(
            *dynamic_cast<const Vulkan::Buffer::Config*>(desc),
            m_buffers.size()
        );

        // Generate an empty handle through the renderer
        auto handle = m_driver->CreateBuffer(desc->name);

        // Push a copy into the buffer array
        m_buffers.push_back(handle);

        // And return a reference to the copy
        return m_buffers.back();
    }

    const ResourceHandle<Aurion::RenderTarget>& RenderGraph::CreateRenderTarget(const Aurion::RenderTarget::Config* desc)
    {
        // Cache the config structure for later processing
        m_render_target_configs.emplace_back(
            *dynamic_cast<const Vulkan::RenderTarget::Config*>(desc),
            m_render_targets.size()
        );

        // Generate an empty handle through the renderer
        auto handle = m_driver->CreateRenderTarget(desc->name);

        // Push a copy into the render target array
        m_render_targets.push_back(handle);

        // And return a reference to the copy
        return m_render_targets.back();
    }

    void RenderGraph::AddPass(const Aurion::RenderPass::Config* desc)
    {
        m_pass_descriptions.push_back(desc);
    }

    void RenderGraph::Compile()
    {
        // First, build the dependency graph, cache for recompilation
        m_dependency_graph = BuildDependencyGraph();

        // Cull any passes that don't contribute to the exported image
        auto cull_mask = CullPasses();

        // Then determine the pass execution order
        auto exec_order = TopologicallySortPasses(cull_mask);

        AURION_TRACE("[Render Graph] Execution Order:");
        for (auto i : exec_order)
            AURION_TRACE("Pass %d: %s", i, m_pass_descriptions[i]->name.c_str());

        // Alias resources against the pass execution order
        AliasResources(exec_order);
    }

    void RenderGraph::Export(const std::string& render_target, const u32& version)
    {
        // Ensure the exported render target is an actual output of the graph
        for (const auto& pass : m_pass_descriptions)
        {
            auto it = std::ranges::find_if(pass->outputs, [&](const auto& output){
                return output.name == render_target && output.version == version;
            });

            if (it != pass->outputs.end())
                m_export_target_ref = { .name = render_target, .version = version };
        }

        if (m_export_target_ref.name.empty())
        {
            AURION_WARN("[Vulkan Render Graph] Failed to export render target { name: %s, version: %d }: No output exists in the graph");
            return;
        }

        for (const auto& target : m_render_targets)
            if (target->GetName() == render_target)
                m_export_target = target;
    }

    const ResourceHandle<Aurion::RenderTarget>& RenderGraph::GetExportTarget() const
    {
        return m_export_target;
    }

    std::vector<std::vector<u64>> RenderGraph::BuildDependencyGraph() const
    {
        std::vector<std::vector<u64>> dep_graph(m_pass_descriptions.size());

        std::unordered_map<Aurion::RenderPass::ResourceRef, u64> writes{};

        // Figure out which buffers are imported. Only transient buffers will have cached configs
        std::vector<u8> imported_mask(m_buffers.size(), 1);
        for (const auto& [_, index] : m_buffer_configs)
            imported_mask[index] = 0;

        // If a buffer is imported, mark the resource as written to, with the base version
        for (u64 i = 0; i < imported_mask.size(); ++i)
            if (imported_mask[i])
                writes[{std::string(m_buffers[i]->GetName()), 0}] = UINT64_MAX;

        // Then do the same for render targets. Only transient render targets will have cached configs
        imported_mask = std::vector<u8>(m_render_targets.size(), 1);
        for (const auto& [_, index] : m_render_target_configs)
            imported_mask[index] = 0;

        for (u64 i = 0; i < imported_mask.size(); ++i)
            if (imported_mask[i])
                writes[{std::string(m_render_targets[i]->GetName()), 0}] = UINT64_MAX;

        // Then, map every resource to the pass that produces it
        for (u64 i = 0; i < m_pass_descriptions.size(); ++i)
            for (const auto& output : m_pass_descriptions[i]->outputs)
                if (auto [_, success] = writes.try_emplace(output, i); !success && writes.at(output) != UINT64_MAX)
                    throw std::runtime_error("[Vulkan::RenderGraph] Duplicate producer for resource '" + output.name + "', version " + std::to_string(output.version));

        // Then, analyze pass inputs to determine pass dependencies
        for (u64 i = 0; i < m_pass_descriptions.size(); ++i)
        {
            for (const auto& input : m_pass_descriptions[i]->inputs)
            {
                const auto it = writes.find(input);

                // If this resource has no producing passes, the pass is invalid
                if (it == writes.end())
                {
                    throw std::runtime_error(
                        "[Vulkan::RenderGraph] Read of resource '"
                        + input.name
                        + "', version "
                        + std::to_string(input.version)
                        + " with no producer"
                    );
                }

                // Only register passes with edges (resource links) to other passes.
                //  Imported/Registered resources don't generate edges
                if (it->second != UINT64_MAX)
                    dep_graph[i].push_back(it->second);
            }
        }

        return dep_graph;
    }

    std::vector<u8> RenderGraph::CullPasses()
    {
        // If there's no export target, all passes are valid
        if (m_export_target_ref.version == UINT64_MAX)
            return std::vector<u8>(m_dependency_graph.size(), 1u);

        // Figure out which pass produces the exported render target
        u64 final_pass_idx = 0;
        for (; final_pass_idx < m_pass_descriptions.size(); ++final_pass_idx)
        {
            const auto& outputs = m_pass_descriptions[final_pass_idx]->outputs;

            auto it = std::ranges::find_if(outputs, [&](const auto& output){
                return output.name == m_export_target_ref.name && output.version == m_export_target_ref.version;
            });

            if (it != outputs.end())
                break;
        }

        // Walk the dependency graph backward, from the pass producing
        //  the exported render target.
        std::vector<u8> visited(m_dependency_graph.size(), 0);
        std::function<void(const u64&)> walk = [&](const u64& index)
        {
            const std::vector<u64>& deps = m_dependency_graph[index];
            visited[index] = true;

            for (const auto& dep : deps)
                walk(dep);
        };

        // After walking, cull all non-visited nodes
        walk(final_pass_idx);
        return visited;
    }

    std::vector<u64> RenderGraph::TopologicallySortPasses(std::span<u8> mask) const
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

            // Then, recursively process all dependents, if they haven't been culled
            for (const auto& dep : m_dependency_graph[node])
                if (mask[dep])
                    visit(dep);

            // Once all dependents have been processed:
            stack[node] = false; // Remove from the call-stack
            visited[node] = true; // Mark this pass as processed
            execution_order.push_back(node); // Add index to execution sequence
        };

        // Process all unvisited, unculled nodes
        for (size_t i = 0; i < m_pass_descriptions.size(); ++i)
            if (mask[i] && !visited[i]) visit(i);

        // Return the final execution order
        return execution_order;
    }

    void RenderGraph::AliasResources(std::span<u64> execution_order)
    {
        // NOTE: Only transient resources get aliased. Imported resources are ignored
        std::vector<std::pair<u64, u64>> buffer_lifetimes(m_buffer_configs.size(), { UINT64_MAX, UINT64_MAX });
        std::vector<std::pair<u64, u64>> rt_lifetimes(m_render_target_configs.size(), { UINT64_MAX, UINT64_MAX });

        auto determine_lifetime = [&](const u64& idx, const Aurion::RenderPass::ResourceRef& ref)
        {
            // Check against all buffer configs
            for (u64 j = 0; j < m_buffer_configs.size(); ++j)
            {
                if (m_buffer_configs[j].config.name == ref.name)
                {
                    // First time seeing this resource
                    if (buffer_lifetimes[j].first == UINT64_MAX)
                        buffer_lifetimes[j] = {idx, idx};
                    else // Increase lifespan
                        buffer_lifetimes[j].second = idx;
                }
            }

            // And against all render target configs
            for (u64 j = 0; j < m_render_target_configs.size(); ++j)
            {
                if (m_render_target_configs[j].config.name == ref.name)
                {
                    // First time seeing this resource
                    if (rt_lifetimes[j].first == UINT64_MAX)
                        rt_lifetimes[j] = {idx, idx};
                    else // Increase lifespan
                        rt_lifetimes[j].second = idx;
                }
            }
        };

        // Determine Resource Lifetimes
        for (u64 i = 0; i < execution_order.size(); ++i)
        {
            // Figure out which pass we're referencing
            const auto& pass = m_pass_descriptions[execution_order[i]];

            // Process input resources
            for (const auto& input : pass->inputs)
                determine_lifetime(i, input);

            // Process output resources
            for (const auto& output : pass->outputs)
                determine_lifetime(i, output);
        }

        // TODO: Implement resource memory aliasing based on lifetime within the graph

        vk::MemoryRequirements mem_reqs;

        // Create all buffers
        Vulkan::Buffer* buffer = nullptr;
        for (const auto& desc : m_buffer_configs)
        {
            // Grab the blank handle
            auto& target = m_buffers[desc.index];
            buffer = static_cast<Vulkan::Buffer*>(target.Get());

            // Then configure
            buffer->Configure(&desc.config);

            // Get memory requirements
            mem_reqs = buffer->GetMemoryRequirements();

            // Need the driver to allocate and bind device memory
            auto memory = m_driver->AllocateDeviceMemory(
                mem_reqs,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
            );

            buffer->BindDeviceMemory(memory, 0);
        }

        // Create all render targets
        Vulkan::RenderTarget* rt = nullptr;
        for (auto& desc : m_render_target_configs)
        {
            // Grab the blank handle
            auto& target = m_render_targets[desc.index];
            rt = static_cast<Vulkan::RenderTarget*>(target.Get());

            // Truncate all render targets to a single frame in flight.
            if (desc.config.frames_in_flight > 1)
            {
                AURION_WARN("[Vulkan::RenderGraph] Transient Render Targets are limited to 1 in-flight-frame. Truncating Render Target '%s'", desc.config.name.c_str());
                desc.config.frames_in_flight = 1;
            }

            // Then configure
            rt->Configure(&desc.config);

            // Get memory requirements
            mem_reqs = rt->GetMemoryRequirements();

            continue;

            // Need the driver to allocate and bind device memory
            auto memory = m_driver->AllocateDeviceMemory(
                mem_reqs,
                vk::MemoryPropertyFlagBits::eDeviceLocal
            );

            rt->BindDeviceMemory(memory, 0);
        }
    }
}
