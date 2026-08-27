module;

#include <vulkan/vulkan_raii.hpp>
#include <cstdint>
#include <unordered_map>

module Aurion.Vulkan;

namespace Aurion::Vulkan
{
    RenderContext::RenderContext(
            Driver* driver,
            const vk::raii::Device& logical_device,
            const CommandBufferGroup& buffers,
            const u32& max_frame_count
    ) : m_driver(driver), m_logical_device(&logical_device), m_frame_context({})
    {
        m_fences.reserve(max_frame_count);
        m_graphics_buffers.reserve(buffers.graphics.second.size());
        m_compute_buffers.reserve(buffers.compute.second.size());
        m_transfer_buffers.reserve(buffers.transfer.second.size());

        // Set up per-frame fences based on the number of buffers.
        //  Each buffer array is guaranteed to be the same length
        vk::FenceCreateInfo fence_c_info{};
        fence_c_info.flags = vk::FenceCreateFlagBits::eSignaled;
        for (u32 i = 0; i < max_frame_count; i++)
            m_fences.push_back(vk::raii::Fence(*m_logical_device, fence_c_info));

        // Each unique command buffer needs to be promoted to a command list.
        for (auto& buffer : buffers.graphics.second)
            m_graphics_buffers.emplace_back(driver, &buffer, buffers.graphics.first);

        for (auto& buffer : buffers.compute.second)
            m_compute_buffers.emplace_back(driver, &buffer, buffers.compute.first);

        for (auto& buffer : buffers.transfer.second)
            m_transfer_buffers.emplace_back(driver, &buffer, buffers.transfer.first);
    }

    RenderContext::~RenderContext()
    {
        m_logical_device->waitIdle();
    }

    void RenderContext::BeginFrame()
    {
        // Wait for and reset this frame's fence
        const auto fence_result = m_logical_device->waitForFences(*m_fences[m_frame_context.frame_index], vk::True, UINT64_MAX);
        if (fence_result != vk::Result::eSuccess)
            throw std::runtime_error("[Vulkan::RenderContext] Failed to wait for render fence!");

        m_logical_device->resetFences(*m_fences[m_frame_context.frame_index]);
    }

    void RenderContext::Draw(const RenderGraphCompilationResult& graph)
    {
        CommandList& graphics_cmd = m_graphics_buffers[m_frame_context.frame_index];
        // CommandList& compute_cmd = m_compute_buffers[m_frame_context.frame_index];
        // CommandList& transfer_cmd = m_transfer_buffers[m_frame_context.frame_index];

        graphics_cmd.Begin();
        // compute_cmd.Begin();
        // transfer_cmd.Begin();

        // Cache the graph's exported target as the presentable image
        m_present_target = graph.export_target;

        // Set up frame context
        // Only update the render extent if the export target is a valid render target
        if (GPUHandleType(m_present_target.handle) == GPUResourceType::RenderTarget)
        {
            RenderTargetHandle export_handle{};
            export_handle.value = m_present_target.handle.value;
            const RenderTargetData& rt_data = m_driver->GetRenderTargetData(export_handle);

            // Attempt to get the next image index. This will return the frame index for 'offline' images,
            //  and the actual image index for vk::SwapchainKHR images.
            const auto img_idx = m_driver->AcquireNextImage(export_handle, m_frame_context.frame_index);
            if (img_idx == UINT32_MAX)
                return;

            if (rt_data.swapchain.value != 0)
            {
                const SwapchainData& swap_data = m_driver->GetSwapchainData(rt_data.swapchain);
                m_frame_context.render_extent = swap_data.extent;
            } else
                m_frame_context.render_extent = rt_data.desc.image_desc.extent;
        }

        // Execute each pass, inserting resource transition barriers where appropriate
        {
            std::unordered_map<u64, ResourceUsageIntent> resource_intents{};

            for (const auto& pass_idx : graph.execution_order) {
                auto& pass = graph.passes[pass_idx];

                PipelineBarrierGroup barrier_group{};
                std::vector<SubresourceTransition> subresource_transitions{};

                auto process = [&](const RenderGraphResource& res) {
                    const auto& current_intent = resource_intents[res.handle.value];

                    if (current_intent == res.usage) return;

                    // If a state mismatch is found, a barrier must be injected
                    switch (GPUHandleType(res.handle))
                    {
                    case GPUResourceType::Buffer:
                        {
                            BufferBarrier barrier{};
                            barrier.buffer = static_cast<BufferHandle>(res.handle);
                            barrier.offset = 0;
                            barrier.size = 0ull;
                            barrier.src_access = AccessFromUsageIntent(current_intent);
                            barrier.dst_access = AccessFromUsageIntent(res.usage);

                            barrier_group.buffers.push_back(barrier);
                            break;
                        }
                    case GPUResourceType::RenderTarget:
                        {
                            const RenderTargetHandle rt_handle = static_cast<RenderTargetHandle>(res.handle);
                            const RenderTargetData& rt_data = m_driver->GetRenderTargetData(rt_handle);

                            ImageBarrier barrier{};
                            barrier.image = static_cast<TextureHandle>(res.handle);
                            barrier.src_layout = LayoutFromUsageIntent(current_intent);
                            barrier.dst_layout = LayoutFromUsageIntent(res.usage);
                            barrier.src_access = AccessFromUsageIntent(current_intent);
                            barrier.dst_access = AccessFromUsageIntent(res.usage);
                            barrier.subresource_range = rt_data.desc.view_desc.subresource_range;

                            barrier_group.images.push_back(barrier);
                            break;
                        }
                    default: break;
                    }

                    resource_intents[res.handle.value] = res.usage;
                };

                for (const auto& write : pass.writes) process(write);
                for (const auto& read  : pass.reads)  process(read);

                // TODO: Each pass might use a different command buffer/queue (compute, transfer),
                //  but RenderPassDescription doesn't yet declare which one it needs. Every pass is
                //  routed onto the graphics buffer/queue for now.

                // Inject batched barriers before the pass executes
                graphics_cmd.PipelineBarrier(barrier_group);
                pass.on_execute(graphics_cmd, m_frame_context);
            }

            // After all passes execute, transition export target to final layout
            {
                const auto& current_intent = resource_intents[graph.export_target.handle.value];
                const RenderTargetHandle export_rt_handle = static_cast<RenderTargetHandle>(graph.export_target.handle);
                const RenderTargetData& export_rt_data = m_driver->GetRenderTargetData(export_rt_handle);

                ImageBarrier barrier{};
                barrier.image = static_cast<TextureHandle>(graph.export_target.handle);
                barrier.src_access = AccessFromUsageIntent(current_intent);
                barrier.dst_access = PipelineAccess::None;
                barrier.src_layout = LayoutFromUsageIntent(current_intent);
                barrier.dst_layout = LayoutFromUsageIntent(graph.export_target.usage);
                barrier.subresource_range = export_rt_data.desc.view_desc.subresource_range;

                // For the final barrier, the graphics command buffer can be used.
                graphics_cmd.PipelineBarrier({
                    .images = { barrier }
                });
            }
        }

        graphics_cmd.End();
        // compute_cmd.End();
        // transfer_cmd.End();
    }

    void RenderContext::EndFrame()
    {
        CommandList& graphics_cmd = m_graphics_buffers[m_frame_context.frame_index];
        // CommandList& compute_cmd = m_compute_buffers[m_frame_context.frame_index];
        // CommandList& transfer_cmd = m_transfer_buffers[m_frame_context.frame_index];

        // If we're not presenting a render target, simply submit commands and continue the frame loop
        if (m_present_target.handle.value == 0 || m_present_target.usage != ResourceUsageIntent::Present)
        {
            graphics_cmd.Submit({}, {}, *m_fences[m_frame_context.frame_index]);
            // compute_cmd.Submit({}, {}, nullptr);
            // transfer_cmd.Submit({}, {}, nullptr);

            // Increment frame index
            m_frame_context.frame_index = (m_frame_context.frame_index + 1) % m_graphics_buffers.size();

            return;
        }

        // Otherwise, we can assume that the present target is valid, and needs to be synced/presented

        const RenderTargetHandle rt_handle = static_cast<RenderTargetHandle>(m_present_target.handle);
        const RenderTargetData& rt_data = m_driver->GetRenderTargetData(rt_handle);
        const SwapchainData& swap_data = m_driver->GetSwapchainData(rt_data.swapchain);

        vk::SemaphoreSubmitInfo wait_info{};
        wait_info.semaphore = *swap_data.acquire_semaphores[m_frame_context.frame_index];
        wait_info.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

        vk::SemaphoreSubmitInfo signal_info{};
        signal_info.semaphore = *swap_data.present_semaphores[swap_data.image_index];
        signal_info.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

        std::vector<vk::SemaphoreSubmitInfo> sem_waits{ wait_info };
        std::vector<vk::SemaphoreSubmitInfo> sem_signals{ signal_info };
        graphics_cmd.Submit(sem_waits, sem_signals, *m_fences[m_frame_context.frame_index]);
        // compute_cmd.Submit({}, {}, nullptr);
        // transfer_cmd.Submit({}, {}, nullptr);

        std::vector<vk::Semaphore> present_sem_waits{ *swap_data.present_semaphores[swap_data.image_index] };
        auto present_result = graphics_cmd.Present(rt_handle, swap_data.image_index, present_sem_waits);

        // Increment frame index
        m_frame_context.frame_index = (m_frame_context.frame_index + 1) % m_graphics_buffers.size();
    }

    const u32& RenderContext::GetFrameIndex() const
    {
        return m_frame_context.frame_index;
    }

    const Extent& RenderContext::GetRenderExtent() const
    {
        return m_frame_context.render_extent;
    }
}
