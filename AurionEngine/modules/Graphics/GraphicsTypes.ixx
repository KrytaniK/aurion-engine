module;

#include <string>
#include <vector>
#include <functional>

export module Aurion.Graphics:Types;

import Aurion.Utility;
import Aurion.Types;

export namespace Aurion
{
    struct GPUHandle { u64 value = 0; };

    // Generic Type Handles

    struct SurfaceHandle : GPUHandle {};
    struct PipelineHandle : GPUHandle {};
    struct ShaderHandle : GPUHandle {};
    struct BufferHandle : GPUHandle {};
    struct TextureHandle : GPUHandle {};
    struct SamplerHandle : GPUHandle {};
    struct RenderTargetHandle : GPUHandle {};
    struct ResourcePoolHandle : GPUHandle {};
    struct ResourceGroupHandle : GPUHandle {};
    struct ResourceGroupLayoutHandle : GPUHandle {};

    // Debug Handles

    struct QueryPoolHandle : GPUHandle {};

    enum class GPUResourceType : u8
    {
        Unknown = 0,

        // --- Generic Types ---

        Buffer,
        Texture,
        Sampler,
        Shader,
        Pipeline,
        RenderTarget,

        // --- Explicit Buffer Types ---

        StorageBuffer,
        UniformBuffer,
        StorageBufferDynamic,
        UniformBufferDynamic,
        StorageTexelBuffer,
        UniformTexelBuffer,

        // --- Explicit Texture Types ---

        StorageTexture,
        SampledTexture,

        // --- Explicit Sampler Types ---

        CombinedTextureSampler
    };

    enum class SharingMode : u8
    {
        Exclusive = 0,
        Concurrent
    };

    enum class MemoryProperties : u8
    {
        DeviceLocal     = 1 << 0,
        HostVisible     = 1 << 1,
        HostCoherent    = 1 << 2,
        HostCached      = 1 << 3,
        LazilyAllocated = 1 << 4,
        Protected       = 1 << 5,
    };

    enum class BufferUsage : u16
    {
        Unknown = 0,
        TransferSrc         = 1 << 0,
        TransferDst         = 1 << 1,
        UniformTexelBuffer  = 1 << 2,
        StorageTexelBuffer  = 1 << 3,
        UniformBuffer       = 1 << 4,
        StorageBuffer       = 1 << 5,
        IndexBuffer         = 1 << 6,
        VertexBuffer        = 1 << 7,
        IndirectBuffer      = 1 << 8,
        ShaderDeviceAddress = 1 << 9,
        VideoDecodeSrc      = 1 << 10,
        VideoDecodeDst      = 1 << 11,
    };

    enum class TextureType : u8
    {
        OneDimensional = 0,
        TwoDimensional,
        ThreeDimensional,
    };

    enum class TextureTiling : u8
    {
        Optimal = 0,
        Linear
    };

    enum class TextureLayout : u8
    {
        Undefined = 0,
        General,
        ColorAttachment,
        DepthStencilAttachment,
        DepthStencilReadOnly,
        DepthReadOnlyStencilAttachment,
        DepthAttachmentStencilReadOnly,
        DepthAttachment,
        DepthReadOnly,
        StencilAttachment,
        StencilReadOnly,
        ShaderReadOnly,
        TransferSrc,
        TransferDst,
        PreInitialized,
        ReadOnly,
        Attachment,
        RenderingLocalRead,
        VideoEncodeSrc,
        VideoEncodeDst,
        VideoDecodeSrc,
        VideoDecodeDst,
        PresentSrc,
        SharedPresent,
    };

    enum class TextureUsage : u16
    {
        Unknown = 0,
        TransferSrc             = 1 << 0,
        TransferDst             = 1 << 1,
        Sampled                 = 1 << 2,
        Storage                 = 1 << 3,
        ColorAttachment         = 1 << 4,
        DepthStencilAttachment  = 1 << 5,
        TransientAttachment     = 1 << 6,
        InputAttachment         = 1 << 7,
        HostTransfer            = 1 << 8,
        VideoEncodeSrc          = 1 << 9,
        VideoEncodeDst          = 1 << 10,
        VideoDecodeSrc          = 1 << 11,
        VideoDecodeDst          = 1 << 12,
    };

    enum class PipelineType : u8
    {
        Unknown = 0,
        Graphics,
        Compute,
        RayTrace
    };

    enum class PrimitiveTopology : u8
    {
        PointList = 0,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        TriangleFan,
    };

    enum class PolygonMode : u8
    {
        Fill = 0,
        Line,
        Point
    };

    enum class CullMode : u8
    {
        None    = 1 << 1,
        Front   = 1 << 2,
        Back    = 1 << 3
    };

    enum class FrontFace : u8
    {
        Clockwise = 0,
        CounterClockwise
    };

    enum class ShaderLanguage : u8
    {
        Unknown = 0,
        HLSL,
        GLSL,
        SPIRV
    };

    // A bitmask for identifying one or more shader stages
    enum class ShaderStage : u8
    {
        Unknown             = 1 << 0,
        Vertex              = 1 << 1,
        TessellationControl = 1 << 2,
        TessellationEval    = 1 << 3,
        Geometry            = 1 << 4,
        Fragment            = 1 << 5,
        Task                = 1 << 6,
        Mesh                = 1 << 7
    };

    enum class VertexInputRate : u8
    {
        Vertex = 0,
        Instance
    };

    enum class SampleCount : u8
    {
        Sample1     = 1 << 0,
        Sample2     = 1 << 1,
        Sample4     = 1 << 2,
        Sample8     = 1 << 3,
        Sample16    = 1 << 4,
        Sample32    = 1 << 5,
        Sample64    = 1 << 6,
    };

    enum class BlendOp : u8
    {
        Add = 0,
        Subtract,
        SubtractReverse,
        Min,
        Max
    };

    enum class BlendFactor : u8
    {
        Zero = 0,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha
    };

    enum class ColorComponents : u8
    {
        R   = 1 << 0,
        G   = 1 << 1,
        B   = 1 << 2,
        A   = 1 << 3,
    };

    enum class LogicOp : u8
    {
        Clear = 0,
        And,
        AndReversed,
        AndInverted,
        Copy,
        CopyInverted,
        NoOp,
        XOR,
        OR,
        ORReverse,
        ORInverted,
        NOR,
        NAND,
        Equivalent,
        Invert,
        Set
    };

    enum class Format : u8 {
        Undefined = 0,

        // --- Unsigned Normalized (Standard Colors / Textures) ---

        R8_Unorm,
        RG8_Unorm,
        RGBA8_Unorm,
        BGRA8_Unorm,

        // --- Gamma Corrected Colors ---

        RGBA8_Srgb,
        BGRA8_Srgb,

        // --- Signed/Unsigned Integers (IDs, Stencil, Raw Data) ---

        R32_Uint,
        RGBA32_Uint,
        R32_Sint,

        // --- Floats (HDR, Render Targets, Vectors, Compute) ---

        R16_Float,
        RGBA16_Float,
        R32_Float,
        RGB32_Float,
        RGBA32_Float,

        // --- Packed Formats ---

        RGB10A2_Unorm,
        RG11B10_Float,

        // --- Common Depth / Stencil ---

        D32_Float,
        D24_Unorm_S8_Uint,
    };

    enum class ImageAspect : u8
    {
        None        = 1 << 0,
        Color       = 1 << 1,
        Depth       = 1 << 2,
        Stencil     = 1 << 3,
        Metadata    = 1 << 4,
        Plane0      = 1 << 5,
        Plane1      = 1 << 6,
        Plane2      = 1 << 7,
    };

    enum class SwizzleComponent : u8
    {
        Identity = 0,
        Zero,
        One,
        R,
        G,
        B,
        A
    };

    enum class ColorSpace : u8
    {
        SRGBNonLinear = 0
    };

    enum class CompositeAlpha : u8
    {
        Opaque = 0,
        PreMultiplied,
        PostMultiplied,
        Inherit
    };

    enum class PresentMode : u8
    {
        Immediate = 0,
        Mailbox,
        Fifo, // VSync equivelant
        FifoRelaxed,
    };

    enum class ResourcePoolAttributes : u8
    {
        None = 0,
        AllowGroupFree = 1 << 0, // Allows allocated groups to return their allocations to this pool
        AllowGroupOverAllocation = 1 << 1, // Allows a resource pool to allocate more than the maximum set number of groups
        AllowResourceOverAllocation = 1 << 2, // Allows a resource pool to allocate more than the maximum set resources per resource type.
    };

    enum class IndexType : u8
    {
        None = 0,
        Uint8,
        Uint16,
        Uint32,
    };

    enum class PipelineBindPoint : u8
    {
        Graphics = 0,
        Compute,
        RayTracing,
    };

    enum class PipelineAccess : u32
    {
        None = 0,
        IndirectCommandRead         = 1 << 0,
        IndexRead                   = 1 << 1,
        VertexAttributeRead         = 1 << 2,
        UniformRead                 = 1 << 3,
        InputAttachmentRead         = 1 << 4,
        ShaderRead                  = 1 << 5,
        ShaderWrite                 = 1 << 6,
        ColorAttachmentRead         = 1 << 7,
        ColorAttachmentWrite        = 1 << 8,
        DepthStencilAttachmentRead  = 1 << 9,
        DepthStencilAttachmentWrite = 1 << 10,
        TransferRead                = 1 << 11,
        TransferWrite               = 1 << 12,
        HostRead                    = 1 << 13,
        HostWrite                   = 1 << 14,
        MemoryRead                  = 1 << 15,
        MemoryWrite                 = 1 << 16,
        ShaderSampledRead           = 1 << 17,
        ShaderSampledWrite          = 1 << 18,
        ShaderStorageRead           = 1 << 19,
        ShaderStorageWrite          = 1 << 20,
        VideoEncodeRead             = 1 << 21,
        VideoEncodeWrite            = 1 << 22,
        VideoDecodeRead             = 1 << 23,
        VideoDecodeWrite            = 1 << 24
    };

    enum class PipelineStage : u32
    {
        None = 0,
        TopOfPipe = 1 << 0,
        DrawIndirect,
        VertexInput,
        VertexShader,
        TessellationControlShader,
        TessellationEvalShader,
        GeometryShader,
        FragmentShader,
        FragmentTestEarly,
        FragmentTestLate,
        ColorAttachmentOutput,
        ComputeShader,
        AllTransfer,
        Transfer,
        BottomOfPipe,
        Host,
        AllGraphics,
        ALlCommands,
        Copy,
        Resolve,
        Blit,
        Clear,
        IndexInput,
        VertexAttributeInput,
        PreRasterShaders,
        VideoEncode,
        VideoDecode,
        RayTracingShader,
        TaskShader,
        MeshShader,
    };

    // Explicit Flag instantiations

    template<> struct EnableBitwiseFlags<ImageAspect> : std::true_type {};
    template struct Flags<ImageAspect>;

    template<> struct EnableBitwiseFlags<BufferUsage> : std::true_type {};
    template struct Flags<BufferUsage>;

    template<> struct EnableBitwiseFlags<TextureUsage> : std::true_type {};
    template struct Flags<TextureUsage>;

    template<> struct EnableBitwiseFlags<MemoryProperties> : std::true_type {};
    template struct Flags<MemoryProperties>;

    template<> struct EnableBitwiseFlags<CullMode> : std::true_type {};
    template struct Flags<CullMode>;

    template<> struct EnableBitwiseFlags<ColorComponents> : std::true_type {};
    template struct Flags<ColorComponents>;

    template<> struct EnableBitwiseFlags<ShaderStage> : std::true_type {};
    template struct Flags<ShaderStage>;

    template<> struct EnableBitwiseFlags<ResourcePoolAttributes> : std::true_type {};
    template struct Flags<ResourcePoolAttributes>;

    template<> struct EnableBitwiseFlags<PipelineAccess> : std::true_type {};
    template struct Flags<PipelineAccess>;

    struct Extent
    {
        u32 width = 0;
        u32 height = 0;
        u32 depth = 1;
    };

    struct Offset
    {
        u32 x = 0;
        u32 y = 0;
        u32 z = 0;
    };

    struct SwizzleComponentMapping
    {
        SwizzleComponent r = SwizzleComponent::Identity;
        SwizzleComponent g = SwizzleComponent::Identity;
        SwizzleComponent b = SwizzleComponent::Identity;
        SwizzleComponent a = SwizzleComponent::Identity;
    };

    struct VertexBinding
    {
        u32 binding = 0;
        u64 stride = 0;
        VertexInputRate input_rate = VertexInputRate::Vertex;
    };

    struct VertexAttribute
    {
        u32 location = 0;
        u32 binding = 0;
        Format format = Format::Undefined;
        u32 offset = 0;
    };

    struct Viewport
    {
        f32 x = 0.0f;
        f32 y = 0.0f;
        f32 width = 0.0f;
        f32 height = 0.0f;
        f32 min_depth = 0.0f;
        f32 max_depth = 0.0f;
    };

    struct Scissor
    {
        u32 offset_x = 0;
        u32 offset_y = 0;
        u32 width = 0;
        u32 height = 0;
    };

    struct ColorBlendAttachment
    {
        BlendFactor src_color_blend_factor = BlendFactor::Zero;
        BlendFactor dst_color_blend_factor = BlendFactor::Zero;
        BlendOp color_blend_op = BlendOp::Add;

        BlendFactor src_alpha_blend_factor = BlendFactor::Zero;
        BlendFactor dst_alpha_blend_factor = BlendFactor::Zero;
        BlendOp alpha_blend_op = BlendOp::Add;

        Flags<ColorComponents> color_write_mask = ColorComponents::R;
        bool enable_blend = false;
    };

    struct ShaderMacro
    {
        std::string key{};
        std::string value{};
    };

    struct ShaderEntryPoint
    {
        ShaderStage stage = ShaderStage::Vertex;
        std::string id{};
    };

    struct SubresourceRange
    {
        Flags<ImageAspect> aspect_mask = ImageAspect::None;
        u32 base_mip_level = 0;
        u32 level_count = 0;
        u32 base_array_layer = 0;
        u32 layer_count = 0;
    };

    struct ResourceGroupBinding
    {
        u32 binding = 0; // The binding location in the shader
        GPUResourceType type = GPUResourceType::Unknown; // The resource kind
        u32 count = 0; // The number of resources at this binding: 1 or N for an array
        Flags<ShaderStage> stage_flags = ShaderStage::Vertex | ShaderStage::Fragment;
    };

    struct ResourceBindingUpdate
    {
        ResourceGroupHandle group = {0}; // The resource group this resource should bind to
        GPUResourceType type = GPUResourceType::Unknown; // The type of resource being bound
        u32 binding = 0; // The binding location in the shader
        u32 first_index = 0; // The index of the first element to update in the shader array
        std::vector<GPUHandle> resources{}; // Handles to each resource to bind
    };

    struct ResourceGroupSize
    {
        GPUResourceType type = GPUResourceType::Unknown;
        u32 count = 0;
    };

    struct RenderPassResource
    {
        std::string name{};
        u32 generation = 0;
    };

    struct FrameContext
    {
        u32 frame_index = 0;
        Extent render_extent{};
    };

    struct BufferCopy
    {
        u64 src_offset = 0;
        u64 dst_offset = 0;
        u64 size = 0;
    };

    struct BufferTextureCopy
    {
        u64 buffer_offset = 0;   // byte offset into the buffer
        u32 buffer_row_length = 0;   // in texels; 0 == tightly packed to region width
        u32 buffer_image_height = 0;   // in texels; 0 == tightly packed to region height
        u32 mip_level = 0;
        u32 array_layer = 0;
        u32 layer_count = 1;
        i32 offset_x = 0;
        i32 offset_y = 0;
        i32 offset_z = 0;
        Extent extent;
    };

    struct ImageBarrier
    {
        TextureHandle image{};
        Flags<PipelineAccess> src_access = PipelineAccess::None;
        Flags<PipelineAccess> dst_access = PipelineAccess::None;
        TextureLayout src_layout = TextureLayout::Undefined;
        TextureLayout dst_layout = TextureLayout::Undefined;
        SubresourceRange subresource_range{};
    };

    struct BufferBarrier
    {
        BufferHandle buffer{};
        Flags<PipelineAccess> src_access = PipelineAccess::None;
        Flags<PipelineAccess> dst_access = PipelineAccess::None;
        u64 offset = 0;
        u64 size = 0;
    };

    struct MemoryBarrier
    {
        Flags<PipelineAccess> src_access = PipelineAccess::None;
        Flags<PipelineAccess> dst_access = PipelineAccess::None;
    };

    struct PipelineBarrierGroup
    {
        std::vector<MemoryBarrier> memory{};
        std::vector<ImageBarrier> images{};
        std::vector<BufferBarrier> buffers{};
    };

    struct ICommandList;
    typedef std::function<void(ICommandList&, const FrameContext&)> RenderPassExecFn;
}