module;

#include <memory>
#include <vector>
#include <span>
#include <string_view>

export module Aurion.Graphics:Interface;

import Aurion.Assets;
import Aurion.Types;

import :Config;
import :Types;

export namespace Aurion
{
    // Base interface for graphics-related resources
    struct IGraphicsAsset : IAsset
    {
        [[nodiscard]] virtual GPUResourceType GetType() const = 0;
        [[nodiscard]] virtual const GPUHandle& GetHandle() const = 0;
    };

    // Base interface for working with per-frame graphics resources
    struct ICommandList
    {
        virtual ~ICommandList() = default;

        // --- Command Recording ---

        virtual void BeginRecording(const RenderTargetHandle& target) = 0;
        virtual void EndRecording() = 0;

        // --- Pipeline + Dynamic State ---

        virtual void BindPipeline(const PipelineBindPoint& bind_point, const PipelineHandle& pipeline) = 0;

        virtual void SetViewport(const Viewport& viewport) = 0;
        virtual void SetScissor(const Scissor& scissor) = 0;

        virtual void SetViewports(const u32& first, std::span<Viewport> viewports) = 0;
        virtual void SetScissors(const u32& first, std::span<Scissor> viewports) = 0;

        virtual void SetBlendConstants(const std::array<f32, 4>& rgba) = 0;
        virtual void SetStencilReference(const u32& ref) = 0;
        virtual void SetDepthBias(const f32& constant, const f32& clamp, const f32& slope) = 0;

        // --- Resource Binding ---

        virtual void BindResourceGroup(
            const PipelineBindPoint& bind_point, const PipelineHandle& pipeline,
            const u32& group, const ResourceGroupHandle& handle) = 0;
        virtual void BindResourceGroups(
            const PipelineBindPoint& bind_point, const PipelineHandle& pipeline,
            const u32& first_group, std::span<ResourceGroupHandle> group,
            std::span<const u32> dynamic_offsets) = 0;

        virtual void BindVertexBuffer(const u32& binding, const BufferHandle& buffer, const u32& offset) = 0;
        virtual void BindVertexBuffers(const u32& first_binding, std::span<BufferHandle> buffers, std::span<u32> offsets) = 0;
        virtual void BindIndexBuffer(const BufferHandle& buffer, const u32& offset, const IndexType& type) = 0;

        // --- Draw Commands ---

        virtual void Draw(const u32& vertex_count, const u32& instance_count, const u32& first_vertex, const u32& first_instance) = 0;
        virtual void DrawIndexed(const u32& index_count, const u32& instance_count, const u32& first_index, const i32& vertex_offset, const u32& first_instance) = 0;
        virtual void DrawIndirect(const BufferHandle& args, const u64& offset, const u32& draw_count, const u32& stride) = 0;
        virtual void DrawIndexedIndirect(const BufferHandle& args, const u64& offset, const u32& draw_count, const u32& stride) = 0;

        // --- Compute Commands ---

        virtual void Dispatch(const u32& group_count_x, const u32& group_count_y, const u32& group_count_z) = 0;
        virtual void DispatchIndirect(const BufferHandle& args, const u64& offset) = 0;

        // --- Transfer Commands ---

        virtual void CopyBuffer(const BufferHandle& src, const BufferHandle& dst, std::span<const BufferCopy> regions) = 0;
        virtual void CopyBufferToTexture(const BufferHandle& src, const TextureHandle& dst, std::span<const BufferTextureCopy> regions) = 0;
        virtual void CopyTextureToBuffer(const TextureHandle& src, const BufferHandle& dst, std::span<const BufferTextureCopy> regions) = 0;

        // --- Transition Commands ---

        virtual void TransitionSubresource(const SubresourceTransition& transition) = 0;

        virtual void PipelineBarrier(const PipelineBarrierGroup& barriers) = 0;

        // --- Debug Commands ---

        virtual void PushDebugGroup(const std::string_view& label, const u32& rgba) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(const std::string_view& label, const u32& rgba) = 0;
        virtual void WriteTimestamp(const QueryPoolHandle& pool, const u32& query_index) = 0;
        virtual void BeginQuery(const QueryPoolHandle& pool, const u32& query_index) = 0;
        virtual void EndQuery(const QueryPoolHandle& pool, const u32& query_index) = 0;
    };

    struct IRenderContext
    {
        virtual ~IRenderContext() = default;

        virtual void BeginFrame() = 0;
        virtual void Draw(const RenderGraphCompilationResult& graph) = 0;
        virtual void EndFrame() = 0;

        [[nodiscard]] virtual const u32& GetFrameIndex() const = 0;
        [[nodiscard]] virtual const Extent& GetRenderExtent() const = 0;
    };

    // Base driver interface for graphics-related operations
    struct IGraphicsDriver
    {
        virtual ~IGraphicsDriver() = default;

        // --- Render Context ---

        [[nodiscard]] virtual std::shared_ptr<IRenderContext> CreateRenderContext(const PresentMode& present_mode) = 0;

        // --- Single Resource Allocation ---

        [[nodiscard]] virtual SurfaceHandle CreateSurface(const SurfaceDescription& desc) = 0;
        [[nodiscard]] virtual PipelineHandle CreatePipeline(const PipelineDescription& desc) = 0;
        [[nodiscard]] virtual ShaderHandle CreateShader(const ShaderDescription& desc) = 0;
        [[nodiscard]] virtual BufferHandle CreateBuffer(const BufferDescription& desc) = 0;
        [[nodiscard]] virtual TextureHandle CreateTexture(const TextureDescription& desc) = 0;
        [[nodiscard]] virtual TextureViewHandle CreateTextureView(const TextureViewDescription& desc) = 0;
        [[nodiscard]] virtual SamplerHandle CreateSampler(const SamplerDescription& desc) = 0;
        [[nodiscard]] virtual RenderTargetHandle CreateRenderTarget(const RenderTargetDescription& desc) = 0;

        [[nodiscard]] virtual ResourcePoolHandle CreateResourcePool(const ResourcePoolDescription& desc) = 0;
        [[nodiscard]] virtual ResourceGroupLayoutHandle CreateResourceGroupLayout(const ResourceGroupLayoutDescription& desc) = 0;
        [[nodiscard]] virtual ResourceGroupHandle AllocateResourceGroup(const ResourcePoolHandle& pool, const ResourceGroupLayoutHandle& layout) = 0;

        // --- Batch Resource Allocation ---

        [[nodiscard]] virtual std::vector<PipelineHandle> CreatePipelines(std::span<PipelineDescription> descriptions) = 0;
        [[nodiscard]] virtual std::vector<ShaderHandle> CreateShaders(std::span<ShaderDescription> descriptions) = 0;
        [[nodiscard]] virtual std::vector<BufferHandle> CreateBuffers(std::span<BufferDescription> descriptions) = 0;
        [[nodiscard]] virtual std::vector<TextureHandle> CreateTextures(std::span<TextureDescription> descriptions) = 0;
        [[nodiscard]] virtual std::vector<TextureViewHandle> CreateTextureViews(std::span<TextureViewDescription> descriptions) = 0;
        [[nodiscard]] virtual std::vector<SamplerHandle> CreateSamplers(std::span<SamplerDescription> descriptions) = 0;
        [[nodiscard]] virtual std::vector<RenderTargetHandle> CreateRenderTargets(std::span<RenderTargetDescription> descriptions) = 0;

        [[nodiscard]] virtual std::vector<ResourcePoolHandle> CreateResourcePools(std::span<ResourcePoolDescription> descriptions) = 0;
        [[nodiscard]] virtual std::vector<ResourceGroupLayoutHandle> CreateResourceGroupLayouts(std::span<ResourceGroupLayoutDescription> descriptions) = 0;

        [[nodiscard]] virtual std::vector<ResourceGroupHandle> AllocateResourceGroups(const ResourcePoolHandle& pool, const ResourceGroupLayoutHandle& layout, const u32& count) = 0;

        // --- Resource Cleanup ---

        virtual void Release(GPUHandle& handle) = 0;

        // --- Resource & Memory Binding ---

        virtual void UpdateResourceGroupBinding(const ResourceBindingUpdate& update) = 0;
        virtual void UpdateResourceGroupBindings(std::span<ResourceBindingUpdate> updates) = 0;

        [[nodiscard]] virtual ResourceMemoryRequirements QueryMemoryRequirements(const BufferDescription& buffer_desc) = 0;
        [[nodiscard]] virtual ResourceMemoryRequirements QueryMemoryRequirements(const RenderTargetDescription& render_target_desc) = 0;

        [[nodiscard]] virtual ResourceMemoryHandle AllocateResourceMemory(const ResourceMemoryRequirements& requirements) = 0;
        virtual void BindResourceMemory(const GPUHandle& resource, const ResourceMemoryHandle& memory, const u64& offset) = 0;

        // --- Resource Handle Retrieval ---

        [[nodiscard]] virtual ResourceGroupLayoutHandle GetShaderResourceGroupLayout(const ShaderHandle& shader) = 0;

        [[nodiscard]] virtual Extent GetRenderTargetExtent(const RenderTargetHandle& handle) = 0;

        // --- Buffer Operations ---

        virtual void MapBuffer(const BufferHandle& handle, const u32& offset, const u32& size) = 0;
        virtual void WriteToBuffer(const BufferHandle& handle, void* data, const u32& offset, const u32& size) = 0;
        virtual void UnMapBuffer(const BufferHandle& handle) = 0;

        // --- Texture Operations ---

        // --- Sampler Operations ---

        // --- Shader Operations ---

        // --- Pipeline Operations ---

        // --- Render Target Operations ---
    };
}