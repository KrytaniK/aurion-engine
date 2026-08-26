module;

#include <vulkan/vulkan_raii.hpp>
#include <string>
#include <array>
#include <span>
#include <memory>

export module Aurion.Vulkan:CommandList;

import Aurion.Graphics;

import :Types;

export namespace Aurion::Vulkan
{
    class Driver;

    // A thin, cheap-to-construct wrapper around a command buffer and the queue it will be
    //  submitted to. Neither is owned here - both are allocated and kept alive by a QueueFamily,
    //  which outlives any CommandList wrapping them.
    class CommandList : public ICommandList
    {
    public:
        explicit CommandList(const Driver* driver, vk::raii::CommandBuffer* buffer, const QueueData* queue);
        ~CommandList() override;

        // --- Command Recording ---

        void Begin();
        void BeginRecording(const RenderTargetHandle& target) override;
        void EndRecording() override;
        void End();

        // --- Pipeline + Dynamic State ---

        void BindPipeline(const PipelineBindPoint& bind_point, const PipelineHandle& pipeline) override;

        void SetViewport(const Viewport& viewport) override;
        void SetScissor(const Scissor& scissor) override;

        void SetViewports(const u32& first, std::span<Viewport> viewports) override;
        void SetScissors(const u32& first, std::span<Scissor> viewports) override;

        void SetBlendConstants(const std::array<f32, 4>& rgba) override;
        void SetStencilReference(const u32& ref) override;
        void SetDepthBias(const f32& constant, const f32& clamp, const f32& slope) override;

        // --- Resource Binding ---

        void BindResourceGroup(
            const PipelineBindPoint& bind_point, const PipelineHandle& pipeline,
            const u32& group, const ResourceGroupHandle& handle) override;
        void BindResourceGroups(
            const PipelineBindPoint& bind_point, const PipelineHandle& pipeline,
            const u32& first_group, std::span<ResourceGroupHandle> group,
            std::span<const u32> dynamic_offsets) override;

        void BindVertexBuffer(const u32& binding, const BufferHandle& buffer, const u32& offset) override;
        void BindVertexBuffers(const u32& first_binding, std::span<BufferHandle> buffers, std::span<u32> offsets) override;
        void BindIndexBuffer(const BufferHandle& buffer, const u32& offset, const IndexType& type) override;

        // --- Draw Commands ---

        void Draw(const u32& vertex_count, const u32& instance_count, const u32& first_vertex, const u32& first_instance) override;
        void DrawIndexed(const u32& index_count, const u32& instance_count, const u32& first_index, const i32& vertex_offset, const u32& first_instance) override;
        void DrawIndirect(const BufferHandle& args, const u64& offset, const u32& draw_count, const u32& stride) override;
        void DrawIndexedIndirect(const BufferHandle& args, const u64& offset, const u32& draw_count, const u32& stride) override;

        // --- Compute Commands ---

        void Dispatch(const u32& group_count_x, const u32& group_count_y, const u32& group_count_z) override;
        void DispatchIndirect(const BufferHandle& args, const u64& offset) override;

        // --- Transfer Commands ---

        void CopyBuffer(const BufferHandle& src, const BufferHandle& dst, std::span<const BufferCopy> regions) override;
        void CopyBufferToTexture(const BufferHandle& src, const TextureHandle& dst, std::span<const BufferTextureCopy> regions) override;
        void CopyTextureToBuffer(const TextureHandle& src, const BufferHandle& dst, std::span<const BufferTextureCopy> regions) override;

        // --- Transition Commands ---

        void TransitionSubresource(const SubresourceTransition& transition) override;

        void PipelineBarrier(const PipelineBarrierGroup& barriers) override;

        // --- Debug Commands ---

        void PushDebugGroup(const std::string_view& label, const u32& rgba) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(const std::string_view& label, const u32& rgba) override;
        void WriteTimestamp(const QueryPoolHandle& pool, const u32& query_index) override;
        void BeginQuery(const QueryPoolHandle& pool, const u32& query_index) override;
        void EndQuery(const QueryPoolHandle& pool, const u32& query_index) override;

        // --- Submission / Presentation ---
        //  Convenience helpers so the buffer/queue pairing doesn't need to be re-resolved by callers
        //  at the end of a frame.

        void Submit(std::span<const vk::SemaphoreSubmitInfo> wait_semaphores, std::span<const vk::SemaphoreSubmitInfo> signal_semaphores, const vk::Fence& fence = nullptr) const;

        // `image_index` is the swapchain image being presented - acquiring it is not yet this
        //  class's responsibility, so the caller must supply it.
        [[nodiscard]] vk::Result Present(const RenderTargetHandle& target, const u32& image_index, std::span<const vk::Semaphore> wait_semaphores) const;

    private:
        const Driver* m_driver;
        const QueueData* m_queue;
        vk::raii::CommandBuffer* m_buffer;
    };
}