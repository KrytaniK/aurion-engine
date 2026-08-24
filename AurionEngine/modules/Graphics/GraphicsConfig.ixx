module;

#include <string>
#include <vector>
#include <array>

export module Aurion.Graphics:Config;

import Aurion.Window;
import Aurion.Types;

import :Types;

export namespace Aurion
{
    // Pipelines

    struct VertexInputState
    {
        std::vector<VertexBinding> bindings{};
        std::vector<VertexAttribute> attributes{};
    };

    struct InputAssemblyState
    {
        PrimitiveTopology topology = PrimitiveTopology::LineList;
        bool enable_primitive_restart = false;
    };

    struct RasterizationState
    {
        PolygonMode polygon_mode = PolygonMode::Fill;
        Flags<CullMode> cull_mode = CullMode::None;
        FrontFace front_face = FrontFace::Clockwise;
        f32 depth_bias_constant = 0.0f;
        f32 depth_bias_clamp = 0.0f;
        f32 depth_bias_slope = 0.0f;
        f32 line_width = 0.0f;
        bool enable_depth_clamp = false;
        bool enable_rasterizer_discard = false;
        bool enable_depth_bias = false;
    };

    struct ViewportState
    {
        std::vector<Viewport> viewports{};
        std::vector<Scissor> scissors{};
        bool enable_dynamic_viewport = false;
        bool enable_dynamic_scissor = false;
    };

    struct MultisamplingState
    {
        u32* sample_mask = nullptr;
        SampleCount rasterization_samples = SampleCount::Sample1;
        bool enable_sample_shading = false;
        bool enable_alpha_to_coverage = false;
        bool enable_alpha_to_one = false;
    };

    struct ColorBlendState
    {
        std::vector<ColorBlendAttachment> blend_attachments{};
        LogicOp logic_op = LogicOp::Clear;
        bool enable_logic_op = false;
        std::array<f32, 4> blend_constants{};
    };

    struct ColorAttachmentState
    {
        u32 view_mask = 0;
        std::vector<Format> formats{};
        Format depth_format = Format::Undefined;
        Format stencil_format = Format::Undefined;
    };

    struct PipelineDescription
    {
        PipelineType pType = PipelineType::Unknown;
    };

    struct GraphicsPipelineDescription : PipelineDescription
    {
        PipelineType pType = PipelineType::Graphics;

        std::vector<ShaderHandle> shaders{};

        VertexInputState vertex_input{};
        InputAssemblyState input_assembly{};
        RasterizationState rasterization{};
        ViewportState viewport{};
        MultisamplingState multisampling{};
        ColorBlendState color_blending{};
        ColorAttachmentState color_attachment{};
        bool enable_dynamic_rendering = false;
    };

    // Shaders

    struct ShaderDescription
    {
        std::string name{};
        std::string path{};
        ShaderLanguage lang = ShaderLanguage::HLSL;
        std::vector<ShaderMacro> defines{};
        std::vector<ShaderEntryPoint> entry_points{};
        std::vector<ResourceGroupBinding> resource_bindings{};
    };

    struct ResourceGroupLayoutDescription
    {
        std::vector<ResourceGroupBinding> bindings{};
    };

    struct ResourcePoolDescription
    {
        Flags<ResourcePoolAttributes> attributes = ResourcePoolAttributes::None;
        u32 max_groups = 0;
        std::vector<ResourceGroupSize> group_sizes{};
    };

    // Buffers

    struct BufferDescription
    {
        u32 size = 0;
        Flags<ResourceCreateFlags> create_flags = ResourceCreateFlags::None;
        Flags<BufferUsage> usage = BufferUsage::StorageBuffer;
        SharingMode share_mode = SharingMode::Exclusive;
        Flags<MemoryProperties> properties = MemoryProperties::DeviceLocal;
    };

    // Textures

    struct TextureDescription
    {
        Extent extent{};
        u32 mip_levels = 0;
        u32 base_mip_level = 0;
        u32 array_layers = 0;
        u32 base_array_layer = 0;
        TextureType type = TextureType::TwoDimensional;
        Format format = Format::Undefined;
        Flags<ResourceCreateFlags> create_flags = ResourceCreateFlags::None;
        Flags<TextureUsage> usage = TextureUsage::ColorAttachment;
        SharingMode sharing_mode = SharingMode::Exclusive;
        SampleCount samples = SampleCount::Sample1;
        TextureTiling tiling = TextureTiling::Optimal;
        TextureLayout initial_layout = TextureLayout::Undefined;
        TextureLayout final_layout = TextureLayout::ColorAttachment;
    };

    struct TextureViewDescription
    {
        TextureHandle texture{};
        TextureType view_type = TextureType::TwoDimensional;
        Format format = Format::Undefined;
        SubresourceRange subresource_range{};
        SwizzleComponentMapping components{
            .r = SwizzleComponent::Identity,
            .g = SwizzleComponent::Identity,
            .b = SwizzleComponent::Identity,
            .a = SwizzleComponent::Identity,
        };
    };

    // Samplers

    struct SamplerDescription
    {
        Filter mag_filter = Filter::Nearest;
        Filter min_filter = Filter::Nearest;
        SamplerMipmapMode mip_map_mode = SamplerMipmapMode::Nearest;
        SamplerAddressMode address_mode_u = SamplerAddressMode::Repeat;
        SamplerAddressMode address_mode_v = SamplerAddressMode::Repeat;
        SamplerAddressMode address_mode_w = SamplerAddressMode::Repeat;
        BorderColor border_color = BorderColor::OpaqueBlackInt;
        CompareOp compare_op = CompareOp::Always;
        float mip_lod_bias = 0.f;
        float max_anisotropy = 0.f;
        float min_lod = 0.f;
        float max_lod = 1.f;
        bool enable_anisotropy = false;
        bool enable_compare = false;
        bool unnormalize_coordinates = false;
    };

    // Render Targets

    struct SurfaceDescription
    {
        Window* window = nullptr;
        ColorSpace color_space = ColorSpace::SRGBNonLinear;
        PresentMode present_mode = PresentMode::Immediate;
        CompositeAlpha composite_alpha = CompositeAlpha::Opaque;
        bool clipped = false;
    };

    struct RenderTargetDescription
    {
        SurfaceHandle surface{}; // Optional Surface Handle for OS Window Presentation
        TextureDescription image_desc{};
        TextureViewDescription view_desc{};
    };

    // --- Render Graph Configuration ---

    struct RenderPassDescription
    {
        std::string name;
        std::vector<RenderGraphResource> reads{};
        std::vector<RenderGraphResource> writes{};
        RenderPassExecFn on_execute = nullptr;
    };

}