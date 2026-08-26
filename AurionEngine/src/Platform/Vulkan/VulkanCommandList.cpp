module;

#include <vulkan/vulkan_raii.hpp>
#include <AurionLog.h>
#include <vector>
#include <array>
#include <string>
#include <algorithm>

module Aurion.Vulkan;

namespace Aurion::Vulkan
{
    CommandList::CommandList(const Driver* driver, vk::raii::CommandBuffer* buffer, const QueueData* queue)
        : m_driver(driver), m_queue(queue), m_buffer(buffer)
    {}

    CommandList::~CommandList()
    {
    }

    void CommandList::Begin()
    {
        m_buffer->reset();
        m_buffer->begin(vk::CommandBufferBeginInfo{});
    }

    // --- Command Recording ---

    void CommandList::BeginRecording(const RenderTargetHandle& target)
    {
        const RenderTargetData& rt_data = m_driver->GetRenderTargetData(target);

        vk::ImageView view{};
        vk::Extent2D extent{};

        if (rt_data.swapchain.value != 0)
        {
            const SwapchainData& sc_data = m_driver->GetSwapchainData(rt_data.swapchain);
            // NOTE: swapchain image-index acquisition isn't wired up yet - always targets the first image.
            view = *sc_data.views[0];
            extent = vk::Extent2D(sc_data.extent.width, sc_data.extent.height);
        }
        else
        {
            const TextureViewData& view_data = m_driver->GetTextureViewData(rt_data.view);
            view = *view_data.view;
            extent = vk::Extent2D(rt_data.desc.image_desc.extent.width, rt_data.desc.image_desc.extent.height);
        }

        const bool is_depth = static_cast<bool>(rt_data.desc.view_desc.subresource_range.aspect_mask & (ImageAspect::Depth | ImageAspect::Stencil));

        vk::RenderingAttachmentInfo attachment{};
        attachment.imageView = view;
        attachment.imageLayout = is_depth ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;
        attachment.loadOp = vk::AttachmentLoadOp::eClear;
        attachment.storeOp = vk::AttachmentStoreOp::eStore;
        attachment.clearValue = is_depth
            ? vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0))
            : vk::ClearValue(vk::ClearColorValue(std::array{0.0f, 0.0f, 0.0f, 1.0f}));

        vk::RenderingInfo rendering_info{};
        rendering_info.renderArea = vk::Rect2D({0, 0}, extent);
        rendering_info.layerCount = 1;

        if (is_depth)
            rendering_info.pDepthAttachment = &attachment;
        else
        {
            rendering_info.colorAttachmentCount = 1;
            rendering_info.pColorAttachments = &attachment;
        }

        m_buffer->beginRendering(rendering_info);
    }

    void CommandList::EndRecording()
    {
        m_buffer->endRendering();
    }

    void CommandList::End()
    {
        m_buffer->end();
    }

    // --- Pipeline + Dynamic State ---

    void CommandList::BindPipeline(const PipelineBindPoint& bind_point, const PipelineHandle& pipeline)
    {
        const PipelineData& data = m_driver->GetPipelineData(pipeline);
        m_buffer->bindPipeline(ToVulkanPipelineBindPoint(bind_point), *data.pipeline);
    }

    void CommandList::SetViewport(const Viewport& viewport)
    {
        const vk::Viewport vp(viewport.x, viewport.y, viewport.width, viewport.height, viewport.min_depth, viewport.max_depth);
        m_buffer->setViewport(0, vp);
    }

    void CommandList::SetScissor(const Scissor& scissor)
    {
        const vk::Rect2D rect({static_cast<i32>(scissor.offset_x), static_cast<i32>(scissor.offset_y)}, {scissor.width, scissor.height});
        m_buffer->setScissor(0, rect);
    }

    void CommandList::SetViewports(const u32& first, std::span<Viewport> viewports)
    {
        std::vector<vk::Viewport> vk_viewports{};
        vk_viewports.reserve(viewports.size());
        for (const auto& v : viewports)
            vk_viewports.emplace_back(v.x, v.y, v.width, v.height, v.min_depth, v.max_depth);

        m_buffer->setViewport(first, vk_viewports);
    }

    void CommandList::SetScissors(const u32& first, std::span<Scissor> scissors)
    {
        std::vector<vk::Rect2D> vk_scissors{};
        vk_scissors.reserve(scissors.size());
        for (const auto& s : scissors)
            vk_scissors.emplace_back(vk::Offset2D(static_cast<i32>(s.offset_x), static_cast<i32>(s.offset_y)), vk::Extent2D(s.width, s.height));

        m_buffer->setScissor(first, vk_scissors);
    }

    void CommandList::SetBlendConstants(const std::array<f32, 4>& rgba)
    {
        m_buffer->setBlendConstants(rgba);
    }

    void CommandList::SetStencilReference(const u32& ref)
    {
        m_buffer->setStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, ref);
    }

    void CommandList::SetDepthBias(const f32& constant, const f32& clamp, const f32& slope)
    {
        m_buffer->setDepthBias(constant, clamp, slope);
    }

    // --- Resource Binding ---

    void CommandList::BindResourceGroup(
        const PipelineBindPoint& bind_point, const PipelineHandle& pipeline,
        const u32& group, const ResourceGroupHandle& handle)
    {
        const PipelineData& pipeline_data = m_driver->GetPipelineData(pipeline);
        const ResourceGroupData& group_data = m_driver->GetResourceGroupData(handle);

        m_buffer->bindDescriptorSets(ToVulkanPipelineBindPoint(bind_point), *pipeline_data.layout, group, *group_data.set, {});
    }

    void CommandList::BindResourceGroups(
        const PipelineBindPoint& bind_point, const PipelineHandle& pipeline,
        const u32& first_group, std::span<ResourceGroupHandle> group,
        std::span<const u32> dynamic_offsets)
    {
        const PipelineData& pipeline_data = m_driver->GetPipelineData(pipeline);

        std::vector<vk::DescriptorSet> sets{};
        sets.reserve(group.size());
        for (const auto& g : group)
            sets.push_back(*m_driver->GetResourceGroupData(g).set);

        m_buffer->bindDescriptorSets(ToVulkanPipelineBindPoint(bind_point), *pipeline_data.layout, first_group, sets, dynamic_offsets);
    }

    void CommandList::BindVertexBuffer(const u32& binding, const BufferHandle& buffer, const u32& offset)
    {
        const BufferData& data = m_driver->GetBufferData(buffer);
        m_buffer->bindVertexBuffers(binding, *data.buffer, static_cast<vk::DeviceSize>(offset));
    }

    void CommandList::BindVertexBuffers(const u32& first_binding, std::span<BufferHandle> buffers, std::span<u32> offsets)
    {
        std::vector<vk::Buffer> vk_buffers{};
        vk_buffers.reserve(buffers.size());
        for (const auto& b : buffers)
            vk_buffers.push_back(*m_driver->GetBufferData(b).buffer);

        std::vector<vk::DeviceSize> vk_offsets{};
        vk_offsets.reserve(offsets.size());
        for (const auto& o : offsets)
            vk_offsets.push_back(static_cast<vk::DeviceSize>(o));

        m_buffer->bindVertexBuffers(first_binding, vk_buffers, vk_offsets);
    }

    void CommandList::BindIndexBuffer(const BufferHandle& buffer, const u32& offset, const IndexType& type)
    {
        const BufferData& data = m_driver->GetBufferData(buffer);
        m_buffer->bindIndexBuffer(*data.buffer, static_cast<vk::DeviceSize>(offset), ToVulkanIndexType(type));
    }

    // --- Draw Commands ---

    void CommandList::Draw(const u32& vertex_count, const u32& instance_count, const u32& first_vertex, const u32& first_instance)
    {
        m_buffer->draw(vertex_count, instance_count, first_vertex, first_instance);
    }

    void CommandList::DrawIndexed(const u32& index_count, const u32& instance_count, const u32& first_index, const i32& vertex_offset, const u32& first_instance)
    {
        m_buffer->drawIndexed(index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void CommandList::DrawIndirect(const BufferHandle& args, const u64& offset, const u32& draw_count, const u32& stride)
    {
        const BufferData& data = m_driver->GetBufferData(args);
        m_buffer->drawIndirect(*data.buffer, offset, draw_count, stride);
    }

    void CommandList::DrawIndexedIndirect(const BufferHandle& args, const u64& offset, const u32& draw_count, const u32& stride)
    {
        const BufferData& data = m_driver->GetBufferData(args);
        m_buffer->drawIndexedIndirect(*data.buffer, offset, draw_count, stride);
    }

    // --- Compute Commands ---

    void CommandList::Dispatch(const u32& group_count_x, const u32& group_count_y, const u32& group_count_z)
    {
        m_buffer->dispatch(group_count_x, group_count_y, group_count_z);
    }

    void CommandList::DispatchIndirect(const BufferHandle& args, const u64& offset)
    {
        const BufferData& data = m_driver->GetBufferData(args);
        m_buffer->dispatchIndirect(*data.buffer, offset);
    }

    // --- Transfer Commands ---

    void CommandList::CopyBuffer(const BufferHandle& src, const BufferHandle& dst, std::span<const BufferCopy> regions)
    {
        const BufferData& src_data = m_driver->GetBufferData(src);
        const BufferData& dst_data = m_driver->GetBufferData(dst);

        std::vector<vk::BufferCopy> vk_regions{};
        vk_regions.reserve(regions.size());
        for (const auto& r : regions)
            vk_regions.emplace_back(r.src_offset, r.dst_offset, r.size);

        m_buffer->copyBuffer(*src_data.buffer, *dst_data.buffer, vk_regions);
    }

    void CommandList::CopyBufferToTexture(const BufferHandle& src, const TextureHandle& dst, std::span<const BufferTextureCopy> regions)
    {
        const BufferData& src_data = m_driver->GetBufferData(src);
        const vk::Image dst_image = m_driver->ResolveImage(dst);

        std::vector<vk::BufferImageCopy> vk_regions{};
        vk_regions.reserve(regions.size());
        for (const auto& r : regions)
        {
            vk::BufferImageCopy copy{};
            copy.bufferOffset = r.buffer_offset;
            copy.bufferRowLength = r.buffer_row_length;
            copy.bufferImageHeight = r.buffer_image_height;
            copy.imageSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, r.mip_level, r.array_layer, std::max<u32>(r.layer_count, 1));
            copy.imageOffset = vk::Offset3D(r.offset_x, r.offset_y, r.offset_z);
            copy.imageExtent = vk::Extent3D(r.extent.width, r.extent.height, r.extent.depth);
            vk_regions.push_back(copy);
        }

        // Assumes the render graph has already transitioned `dst` into TransferDst layout
        //  (see LayoutFromUsageIntent(ResourceUsageIntent::TransferDst)).
        m_buffer->copyBufferToImage(*src_data.buffer, dst_image, vk::ImageLayout::eTransferDstOptimal, vk_regions);
    }

    void CommandList::CopyTextureToBuffer(const TextureHandle& src, const BufferHandle& dst, std::span<const BufferTextureCopy> regions)
    {
        const vk::Image src_image = m_driver->ResolveImage(src);
        const BufferData& dst_data = m_driver->GetBufferData(dst);

        std::vector<vk::BufferImageCopy> vk_regions{};
        vk_regions.reserve(regions.size());
        for (const auto& r : regions)
        {
            vk::BufferImageCopy copy{};
            copy.bufferOffset = r.buffer_offset;
            copy.bufferRowLength = r.buffer_row_length;
            copy.bufferImageHeight = r.buffer_image_height;
            copy.imageSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, r.mip_level, r.array_layer, std::max<u32>(r.layer_count, 1));
            copy.imageOffset = vk::Offset3D(r.offset_x, r.offset_y, r.offset_z);
            copy.imageExtent = vk::Extent3D(r.extent.width, r.extent.height, r.extent.depth);
            vk_regions.push_back(copy);
        }

        // Assumes the render graph has already transitioned `src` into TransferSrc layout
        //  (see LayoutFromUsageIntent(ResourceUsageIntent::TransferSrc)).
        m_buffer->copyImageToBuffer(src_image, vk::ImageLayout::eTransferSrcOptimal, *dst_data.buffer, vk_regions);
    }

    // --- Transition Commands ---

    void CommandList::TransitionSubresource(const SubresourceTransition& transition)
    {
        vk::DependencyInfo dep_info{};
        vk::BufferMemoryBarrier2 buffer_barrier{};
        vk::ImageMemoryBarrier2 image_barrier{};

        switch (GPUHandleType(transition.resource))
        {
            case GPUResourceType::Buffer:
            {
                const BufferData& data = m_driver->GetBufferData(static_cast<BufferHandle>(transition.resource));

                buffer_barrier.srcStageMask = ToVulkanPipelineStage2(transition.src_stage);
                buffer_barrier.dstStageMask = ToVulkanPipelineStage2(transition.dst_stage);
                buffer_barrier.srcAccessMask = ToVulkanAccessFlags2(transition.src_access);
                buffer_barrier.dstAccessMask = ToVulkanAccessFlags2(transition.dst_access);
                buffer_barrier.buffer = *data.buffer;
                buffer_barrier.offset = 0;
                buffer_barrier.size = vk::WholeSize;

                dep_info.bufferMemoryBarrierCount = 1;
                dep_info.pBufferMemoryBarriers = &buffer_barrier;
                break;
            }
            case GPUResourceType::Texture:
            case GPUResourceType::RenderTarget:
            {
                image_barrier.srcStageMask = ToVulkanPipelineStage2(transition.src_stage);
                image_barrier.dstStageMask = ToVulkanPipelineStage2(transition.dst_stage);
                image_barrier.srcAccessMask = ToVulkanAccessFlags2(transition.src_access);
                image_barrier.dstAccessMask = ToVulkanAccessFlags2(transition.dst_access);
                // SubresourceTransition carries no explicit old/new layout - eGeneral is always
                //  valid, if suboptimal for the actual usage.
                image_barrier.oldLayout = vk::ImageLayout::eGeneral;
                image_barrier.newLayout = vk::ImageLayout::eGeneral;
                image_barrier.image = m_driver->ResolveImage(static_cast<TextureHandle>(transition.resource));
                image_barrier.subresourceRange = vk::ImageSubresourceRange(
                    ToVulkanImageAspect(transition.subresource_range.aspect_mask),
                    transition.subresource_range.base_mip_level, std::max<u32>(transition.subresource_range.level_count, 1),
                    transition.subresource_range.base_array_layer, std::max<u32>(transition.subresource_range.layer_count, 1));

                dep_info.imageMemoryBarrierCount = 1;
                dep_info.pImageMemoryBarriers = &image_barrier;
                break;
            }
            default: return;
        }

        m_buffer->pipelineBarrier2(dep_info);
    }

    void CommandList::PipelineBarrier(const PipelineBarrierGroup& barriers)
    {
        std::vector<vk::MemoryBarrier2> mem_barriers{};
        mem_barriers.reserve(barriers.memory.size());
        for (const auto& b : barriers.memory)
        {
            vk::MemoryBarrier2 barrier{};
            barrier.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands;
            barrier.dstStageMask = vk::PipelineStageFlagBits2::eAllCommands;
            barrier.srcAccessMask = ToVulkanAccessFlags2(b.src_access);
            barrier.dstAccessMask = ToVulkanAccessFlags2(b.dst_access);
            mem_barriers.push_back(barrier);
        }

        std::vector<vk::BufferMemoryBarrier2> buf_barriers{};
        buf_barriers.reserve(barriers.buffers.size());
        for (const auto& b : barriers.buffers)
        {
            const BufferData& data = m_driver->GetBufferData(b.buffer);

            vk::BufferMemoryBarrier2 barrier{};
            barrier.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands;
            barrier.dstStageMask = vk::PipelineStageFlagBits2::eAllCommands;
            barrier.srcAccessMask = ToVulkanAccessFlags2(b.src_access);
            barrier.dstAccessMask = ToVulkanAccessFlags2(b.dst_access);
            barrier.buffer = *data.buffer;
            barrier.offset = b.offset;
            barrier.size = b.size == 0 ? vk::WholeSize : b.size;
            buf_barriers.push_back(barrier);
        }

        // Barriers built by the render graph don't carry explicit pipeline stages (see
        //  ImageBarrier/BufferBarrier in GraphicsTypes.ixx) - conservatively synchronize against
        //  every stage rather than guessing one from the access mask.
        std::vector<vk::ImageMemoryBarrier2> img_barriers{};
        img_barriers.reserve(barriers.images.size());
        for (const auto& b : barriers.images)
        {
            vk::ImageMemoryBarrier2 barrier{};
            barrier.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands;
            barrier.dstStageMask = vk::PipelineStageFlagBits2::eAllCommands;
            barrier.srcAccessMask = ToVulkanAccessFlags2(b.src_access);
            barrier.dstAccessMask = ToVulkanAccessFlags2(b.dst_access);
            barrier.oldLayout = ToVulkanImageLayout(b.src_layout);
            barrier.newLayout = ToVulkanImageLayout(b.dst_layout);
            barrier.image = m_driver->ResolveImage(b.image);
            barrier.subresourceRange = vk::ImageSubresourceRange(
                ToVulkanImageAspect(b.subresource_range.aspect_mask),
                b.subresource_range.base_mip_level, std::max<u32>(b.subresource_range.level_count, 1),
                b.subresource_range.base_array_layer, std::max<u32>(b.subresource_range.layer_count, 1));
            img_barriers.push_back(barrier);
        }

        if (mem_barriers.empty() && buf_barriers.empty() && img_barriers.empty())
            return;

        vk::DependencyInfo dep_info{};
        dep_info.memoryBarrierCount = static_cast<u32>(mem_barriers.size());
        dep_info.pMemoryBarriers = mem_barriers.data();
        dep_info.bufferMemoryBarrierCount = static_cast<u32>(buf_barriers.size());
        dep_info.pBufferMemoryBarriers = buf_barriers.data();
        dep_info.imageMemoryBarrierCount = static_cast<u32>(img_barriers.size());
        dep_info.pImageMemoryBarriers = img_barriers.data();

        m_buffer->pipelineBarrier2(dep_info);
    }

    // --- Debug Commands ---

    void CommandList::PushDebugGroup(const std::string_view& label, const u32& rgba)
    {
        const std::string label_str(label);

        vk::DebugUtilsLabelEXT info{};
        info.pLabelName = label_str.c_str();
        info.color = ToDebugColor(rgba);

        m_buffer->beginDebugUtilsLabelEXT(info);
    }

    void CommandList::PopDebugGroup()
    {
        m_buffer->endDebugUtilsLabelEXT();
    }

    void CommandList::InsertDebugMarker(const std::string_view& label, const u32& rgba)
    {
        const std::string label_str(label);

        vk::DebugUtilsLabelEXT info{};
        info.pLabelName = label_str.c_str();
        info.color = ToDebugColor(rgba);

        m_buffer->insertDebugUtilsLabelEXT(info);
    }

    void CommandList::WriteTimestamp(const QueryPoolHandle& pool, const u32& query_index)
    {
        // No query pool allocation exists on the driver yet - nothing to resolve `pool` against.
        AURION_ERROR("[Vulkan::CommandList] WriteTimestamp: query pools are not yet supported.");
    }

    void CommandList::BeginQuery(const QueryPoolHandle& pool, const u32& query_index)
    {
        AURION_ERROR("[Vulkan::CommandList] BeginQuery: query pools are not yet supported.");
    }

    void CommandList::EndQuery(const QueryPoolHandle& pool, const u32& query_index)
    {
        AURION_ERROR("[Vulkan::CommandList] EndQuery: query pools are not yet supported.");
    }

    // --- Submission / Presentation ---

    void CommandList::Submit(std::span<const vk::SemaphoreSubmitInfo> wait_semaphores, std::span<const vk::SemaphoreSubmitInfo> signal_semaphores, const vk::Fence& fence) const
    {
        vk::CommandBufferSubmitInfo cmd_info{};
        cmd_info.commandBuffer = **m_buffer;

        vk::SubmitInfo2 submit_info{};
        submit_info.waitSemaphoreInfoCount = static_cast<u32>(wait_semaphores.size());
        submit_info.pWaitSemaphoreInfos = wait_semaphores.data();
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &cmd_info;
        submit_info.signalSemaphoreInfoCount = static_cast<u32>(signal_semaphores.size());
        submit_info.pSignalSemaphoreInfos = signal_semaphores.data();

        m_queue->handle.submit2(submit_info, fence);
    }

    vk::Result CommandList::Present(const RenderTargetHandle& target, const u32& image_index, std::span<const vk::Semaphore> wait_semaphores) const
    {
        const RenderTargetData& rt_data = m_driver->GetRenderTargetData(target);

        if (rt_data.swapchain.value == 0)
        {
            AURION_ERROR("[Vulkan::CommandList] Present: target is not swapchain-backed.");
            return vk::Result::eErrorUnknown;
        }

        const vk::SwapchainKHR swapchain = *m_driver->GetSwapchainData(rt_data.swapchain).swapchain;

        vk::PresentInfoKHR info{};
        info.waitSemaphoreCount = static_cast<u32>(wait_semaphores.size());
        info.pWaitSemaphores = wait_semaphores.data();
        info.swapchainCount = 1;
        info.pSwapchains = &swapchain;
        info.pImageIndices = &image_index;

        return m_queue->handle.presentKHR(info);
    }
}
