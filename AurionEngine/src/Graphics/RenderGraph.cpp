module;

#include <AurionLog.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <cstdint>

module Aurion.Graphics;

namespace Aurion
{
    RenderGraph::RenderGraph(const std::shared_ptr<IGraphicsDriver>& driver)
        : m_graphics_driver(driver), m_export_target({nullptr, 0}), m_buffer_descriptions({}), m_render_target_descriptions({})
    {
        m_buffer_descriptions.reserve(8);
        m_render_target_descriptions.reserve(8);
    }

    RenderGraph::~RenderGraph()
    {

    }

    const VirtualHandle& RenderGraph::ImportResource(const std::string_view& name, const GPUHandle& handle)
    {
        u64 resource_index = m_resources.size();
        m_resources.emplace_back(handle, std::string(name), resource_index, UINT64_MAX, GPUHandleType(handle));
        return m_resources.back();
    }

    const VirtualHandle& RenderGraph::CreateTransientBuffer(const std::string_view& name, const BufferDescription& desc)
    {
        // Create the virtual handle
        u64 resource_index = m_resources.size();
        m_resources.emplace_back(GPUHandle{}, std::string(name), resource_index, m_buffer_descriptions.size(), GPUResourceType::Buffer);

        // Then, cache the description
        m_buffer_descriptions.push_back(desc);

        return m_resources.back();
    }

    const VirtualHandle& RenderGraph::CreateTransientRenderTarget(const std::string_view& name, const RenderTargetDescription& desc)
    {
        // Create the virtual handle
        u64 resource_index = m_resources.size();
        m_resources.emplace_back(GPUHandle{}, std::string(name), resource_index, m_render_target_descriptions.size(), GPUResourceType::RenderTarget);

        // Then, cache the description
        m_render_target_descriptions.push_back(desc);

        return m_resources.back();
    }

    void RenderGraph::AddPass(const RenderPassDescription& desc)
    {
        m_passes.push_back(desc);
    }

    void RenderGraph::Export(const std::string_view& resource_name, const u64& generation)
    {
        // Make sure a pass has actually declared this resource as a written resource
        bool found = false;
        for (const auto& pass : m_passes)
        {
            auto it = std::ranges::find_if(pass.writes, [&](const auto& write){
                return write.name == m_export_target.first->name && write.generation == m_export_target.second;
            });

            found = found || it != pass.writes.end();
        }

        // An invalid export should not throw an error. The graph will simply run in its entirety
        if (!found)
        {
            AURION_ERROR("Failed to export target '%s', generation '%d': The specified resource/version combination is not a written resource of any known pass.", resource_name, generation);
            return;
        }


        for (const auto& resource : m_resources)
            if (resource.name == resource_name)
                m_export_target = std::make_pair(&resource, generation);
    }

    void RenderGraph::Compile()
    {
        // Build a graph of pass dependencies
        auto dep_graph = BuildDependencyGraph();

        // Cull passes that don't contribute to the final image
        auto cull_mask = CullPasses(dep_graph);

        // Determine pass execution order
        auto exec_order = SortPassesTopologically(dep_graph, cull_mask);

        // Determine Resource lifetimes
        auto resource_lts = GetResourceLifetimes(exec_order);

        // Then alias transient resources whose lifetimes don't overlap
        AliasResources(resource_lts);
    }

    void RenderGraph::Execute(const ICommandList& cmd, const FrameContext& ctx)
    {

    }

    const VirtualHandle* RenderGraph::GetExportTarget() const
    {
        return m_export_target.first;
    }

    std::vector<std::vector<u64>> RenderGraph::BuildDependencyGraph() const
    {
        std::vector<std::vector<u64>> graph{};

        std::hash<std::string> hash;
        std::unordered_map<u64, u64> writes{};

        // Map each resource + generation combo to the pass that produces it
        u64 key = UINT64_MAX;
        for (u64 i = 0; i < m_passes.size(); ++i)
        {
            for (const auto& write : m_passes[i].writes)
            {
                key = hash(write.name) + write.generation;
                if (auto [_, success] = writes.try_emplace(key, i); !success)
                    throw std::runtime_error("[Vulkan::RenderGraph] Duplicate producer for resource '" + write.name + "', generation " + std::to_string(write.generation));
            }
        }

        // Then, analyze pass inputs to determine pass dependencies
        for (u64 i = 0; i < m_passes.size(); ++i)
        {
            for (const auto& read : m_passes[i].reads)
            {
                key = hash(read.name) + read.generation;
                const auto it = writes.find(key);

                // If a read resource has no writer, then that resource doesn't exist
                if (it == writes.end())
                    throw std::runtime_error("[Vulkan::RenderGraph] Read of resource '" + read.name + "', generation " + std::to_string(read.generation) + " with no producer");

                // Only register passes with edges (write->read) to other passes
                graph[i].push_back(it->second);
            }
        }

        return std::move(graph);
    }

    std::vector<u64> RenderGraph::CullPasses(std::span<std::vector<u64>> graph) const
    {
        // If there's no export target, all passes are valid
        if (m_export_target.first->resource_index == UINT64_MAX)
            return std::vector<u64>(graph.size(), 1u);

        // Figure out which pass produces the exported render target
        u64 final_pass_idx = 0;
        for (; final_pass_idx < m_passes.size(); ++final_pass_idx)
        {
            const auto& writes = m_passes[final_pass_idx].writes;

            auto it = std::ranges::find_if(writes, [&](const auto& write){
                return write.name == m_export_target.first->name && write.generation == m_export_target.second;
            });

            if (it != writes.end())
                break;
        }

        // Walk the dependency graph backward, from the pass producing
        //  the exported render target.
        std::vector<u64> visited(graph.size(), 0);
        std::function<void(const u64&)> walk = [&](const u64& index)
        {
            const std::vector<u64>& deps = graph[index];
            visited[index] = true;

            for (const auto& dep : deps)
                walk(dep);
        };

        // After walking, cull all non-visited nodes
        walk(final_pass_idx);
        return visited;
    }

    std::vector<u64> RenderGraph::SortPassesTopologically(std::span<std::vector<u64>> graph, std::span<u64> mask) const
    {
        std::vector<u64> execution_order{};

        // Uses depth-first search to compute a valid execution sequence, without cycles.
        std::vector<bool> visited(m_passes.size(), false);
        std::vector<bool> stack(m_passes.size(), false);

        std::function<void(u64)> visit = [&](const u64& node)
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
            for (const auto& dep : graph[node])
                if (mask[dep])
                    visit(dep);

            // Once all dependents have been processed:
            stack[node] = false; // Remove from the call-stack
            visited[node] = true; // Mark this pass as processed
            execution_order.push_back(node); // Add index to execution sequence
        };

        // Process all unvisited, unculled nodes
        for (size_t i = 0; i < graph.size(); ++i)
            if (mask[i] && !visited[i]) visit(i);

        // Return the final execution order
        return execution_order;
    }

    std::vector<std::pair<u64, u64>> RenderGraph::GetResourceLifetimes(std::span<u64> execution_order) const
    {
        // NOTE: Only transient resources get aliased. Imported resources are ignored
        std::vector<std::pair<u64, u64>> resource_lts(m_resources.size(), { UINT64_MAX, UINT64_MAX });

        auto track_resource_lifetime = [&](const RenderGraphResource& resource, const u64& pass_idx)
        {
            for (u64 i = 0; i < m_resources.size(); ++i)
            {
                if (m_resources[i].name != resource.name)
                    continue;

                // Track the lifetime of existing resources only
                if (resource_lts[i].first == UINT64_MAX) // First time seeing resource
                    resource_lts[i] = { pass_idx, pass_idx };
                else
                    resource_lts[i].second = pass_idx; // nth time seeing resource, update lifetime
            }
        };

        for (const auto& pass_idx : execution_order)
        {
            auto& pass = m_passes[pass_idx];

            // Track all read resources
            for (const auto& read : pass.reads)
                track_resource_lifetime(read, pass_idx);

            // Track all write resources
            for (const auto& write : pass.writes)
                track_resource_lifetime(write, pass_idx);
        }

        return std::move(resource_lts);
    }

    void RenderGraph::CreateResources()
    {
        for (auto& resource : m_resources)
        {
            // Imported resources don't have description structures
            if (resource.desc_index == UINT64_MAX) continue;

            switch (resource.type)
            {
            case GPUResourceType::Buffer:
                {
                    resource.value = m_graphics_driver->CreateBuffer(m_buffer_descriptions[resource.desc_index]).value;
                    break;
                }
            case GPUResourceType::RenderTarget:
                {
                    resource.value = m_graphics_driver->CreateRenderTarget(m_render_target_descriptions[resource.desc_index]).value;
                    break;
                }
            default:
                {
                    AURION_WARN("[Render Graph] Unsupported Resource '%s' with type '%d'!", resource.name, resource.type);
                }
            }
        }
    }

    void RenderGraph::AliasResources(std::span<std::pair<u64, u64>> lifetimes)
    {
        std::vector<std::vector<u64>> alias_batches(lifetimes.size());

        // First, batch resources with similar types and disjoint lifetimes
        bool is_matching_type = false, is_aliasable = false;
        for (const auto& handle : m_resources)
        {
            // Don't alias imported resources
            if (handle.desc_index == UINT64_MAX)
                continue;

            // Get handle lifetime
            const auto& lifetime = lifetimes[handle.resource_index];

            for (const auto& cmp_handle : m_resources)
            {
                // Don't alias imported resources, or itself
                if (cmp_handle.resource_index == handle.resource_index || cmp_handle.desc_index == UINT64_MAX)
                    continue;

                // Get comparison lifetime
                const auto& cmp_lifetime = lifetimes[cmp_handle.resource_index];

                is_matching_type = cmp_handle.type == handle.type;

                // Two cases where resources do not overlap:
                //  first is used before other, and other is used before first.
                //  In either of these two cases, both resources can alias the same memory.
                is_aliasable = cmp_lifetime.first > lifetime.second || lifetime.first > cmp_lifetime.second;

                // If both conditions are met, the resources could 'potentially' be aliasable.
                if (is_matching_type && is_aliasable)
                    alias_batches[handle.resource_index].push_back(cmp_handle.resource_index);
            }
        }

        // Once batched, query memory requirements, assuming they're all going to be aliased
        std::vector<ResourceMemoryRequirements> mem_reqs(m_resources.size());
        for (const auto& resource : m_resources)
        {
            if (resource.desc_index == UINT64_MAX) continue;

            switch (resource.type)
            {
            case GPUResourceType::Buffer:
                {
                    auto& desc = m_buffer_descriptions[resource.desc_index];
                    desc.create_flags |= ResourceCreateFlags::Aliasable;
                    mem_reqs[resource.resource_index] = m_graphics_driver->QueryMemoryRequirements(desc);
                    break;
                }
            case GPUResourceType::RenderTarget:
                {
                    auto& desc = m_render_target_descriptions[resource.desc_index];
                    desc.image_desc.create_flags |= ResourceCreateFlags::Aliasable;
                    mem_reqs[resource.resource_index] = m_graphics_driver->QueryMemoryRequirements(desc);
                    break;
                }
            default: { break; }
            }
        }

        // Then, out of all potential batch combinations, filter out non-aliasable resources within each batch
        std::vector<bool> alias_mask(alias_batches.size());
        std::vector<u64> sub_batch;
        is_aliasable = true;
        for (u64 i = 0; i < alias_batches.size(); ++i)
        {
            if (alias_mask[i]) continue; // Don't attempt to re-alias a resource
            alias_mask[i] = true; // First, mark this resource as aliased

            // Aggregate memory requirements at the first resource in the batch.
            auto& agg_reqs = mem_reqs[i];

            // It's possible for the current resource to require a dedicated allocation.
            //  In this case, aliasing becomes impossible.
            auto& batch = alias_batches[i];
            if (agg_reqs.requires_dedicated_memory)
            {
                batch.clear();
                continue;
            }

            // If there's potential for aliasing, aliasing must be verified for the entire batch.
            sub_batch.clear();
            // At best, all resources in this batch can be aliased.
            sub_batch.reserve(batch.size());

            // Attempt to alias as many resources as possible within a single batch
            for (const auto& resource : batch)
            {
                if (alias_mask[resource]) continue; // Don't attempt to re-alias a resource
                is_aliasable = true; // Reset alias state. "Aliasable until proven not"

                const auto& cmp_reqs = mem_reqs[resource];

                // First, check for compatible memory types. Incompatible memory types can't be aliased.
                //  It's also possible that this resource requires a dedicated allocation.
                if ((agg_reqs.memory_type_mask & cmp_reqs.memory_type_mask) == 0
                    || cmp_reqs.requires_dedicated_memory) continue;

                const auto& first_lt = lifetimes[resource];

                // Each resource in the batch needs to be compared with other in-batch resources
                //  that are guaranteed to be aliased, ensuring they don't overlap
                for (const auto& subresource : sub_batch)
                {
                    const auto& second_lt = lifetimes[subresource];

                    // If the memory types are compatible, the lifetime must be disjoint from the rest of
                    //  the batch.
                    is_aliasable = (second_lt.first > first_lt.second || first_lt.first > second_lt.second);
                    if (!is_aliasable) break;
                }

                // If we can't alias, move on to the next resource
                if (!is_aliasable) { continue; }

                // Otherwise, add it to the sub-batch
                //  and mark it as aliased, then aggregate the memory requirements
                //  with the batch owner
                sub_batch.push_back(resource);
                alias_mask[resource] = true;
                agg_reqs.size = std::max(agg_reqs.size, cmp_reqs.size);
                agg_reqs.alignment = std::max(agg_reqs.alignment, cmp_reqs.alignment);
                agg_reqs.memory_type_mask &= cmp_reqs.memory_type_mask;
            }

            // Once the sub-batch has been determined, update the alias-batch in-place.
            alias_batches[i] = sub_batch;
        }

        // Once batches have been filtered, resources can be created
        for (auto& resource : m_resources)
        {
            if (resource.desc_index == UINT64_MAX) continue;

            switch (resource.type)
            {
            case GPUResourceType::Buffer:
                {
                    auto& desc = m_buffer_descriptions[resource.desc_index];
                    resource.value = m_graphics_driver->CreateBuffer(desc).value;
                    break;
                }
            case GPUResourceType::RenderTarget:
                {
                    auto& desc = m_render_target_descriptions[resource.desc_index];
                    resource.value = m_graphics_driver->CreateRenderTarget(desc).value;
                    break;
                }
            default: { break; }
            }
        }

        // Then, memory can be allocated and aliased
        alias_mask.clear();
        alias_mask.resize(alias_batches.size());
        for (u64 i = 0; i < alias_batches.size(); ++i)
        {
            if (alias_mask[i]) continue; // If a resource is aliased, don't allocate memory for it

            // Each batch index should contain the aggregate memory requirements for the entire batch
            //  where the batch has been filtered
            const auto& batch_reqs = mem_reqs[i];

            auto memory = m_graphics_driver->AllocateResourceMemory(batch_reqs);

            // Once allocated and bound, mark the entire batch as aliased
            m_graphics_driver->BindResourceMemory(m_resources[i], memory);
            alias_mask[i] = true;
            for (const auto& resource : alias_batches[i])
            {
                m_graphics_driver->BindResourceMemory(m_resources[resource], memory);
                alias_mask[resource] = true;
            }
        }
    }
}
