module;

#include <vulkan/vulkan_raii.hpp>

export module Aurion.Vulkan:Utility;

import Aurion.Graphics;
import Aurion.Types;
import Aurion.Utility;

export namespace Aurion::Vulkan
{
    constexpr vk::Format ToVulkanFormat(const Format& format)
    {
        switch (format)
        {
            case Format::R8_Unorm: return vk::Format::eR8Unorm;
            case Format::RG8_Unorm: return vk::Format::eR8G8Unorm;
            case Format::RGBA8_Unorm: return vk::Format::eR8G8B8A8Unorm;
            case Format::BGRA8_Unorm: return vk::Format::eB8G8R8A8Unorm;
            case Format::RGBA8_Srgb: return vk::Format::eR8G8B8A8Srgb;
            case Format::BGRA8_Srgb: return vk::Format::eB8G8R8A8Srgb;
            case Format::R32_Uint: return vk::Format::eR32Uint;
            case Format::RGBA32_Uint: return vk::Format::eR32G32B32A32Uint;
            case Format::R32_Sint: return vk::Format::eR32Sint;
            case Format::R16_Float: return vk::Format::eR16Sfloat;
            case Format::RGBA16_Float: return vk::Format::eR16G16B16A16Sfloat;
            case Format::R32_Float: return vk::Format::eR32Sfloat;
            case Format::RGB32_Float: return vk::Format::eR32G32B32Sfloat;
            case Format::RGBA32_Float: return vk::Format::eR32G32B32A32Sfloat;
            case Format::RGB10A2_Unorm: return vk::Format::eA2B10G10R10UnormPack32;
            case Format::RG11B10_Float: return vk::Format::eB10G11R11UfloatPack32;
            case Format::D32_Float: return vk::Format::eD32Sfloat;
            case Format::D24_Unorm_S8_Uint: return vk::Format::eD24UnormS8Uint;
            case Format::Undefined:
            default: return vk::Format::eUndefined;
        }
    }

    constexpr vk::BufferUsageFlags ToVulkanBufferUsage(const Flags<BufferUsage>& usage)
    {
        vk::BufferUsageFlags out{};
        if (usage & BufferUsage::TransferSrc) out |= vk::BufferUsageFlagBits::eTransferSrc;
        if (usage & BufferUsage::TransferDst) out |= vk::BufferUsageFlagBits::eTransferDst;
        if (usage & BufferUsage::UniformTexelBuffer) out |= vk::BufferUsageFlagBits::eUniformTexelBuffer;
        if (usage & BufferUsage::StorageTexelBuffer) out |= vk::BufferUsageFlagBits::eStorageTexelBuffer;
        if (usage & BufferUsage::UniformBuffer) out |= vk::BufferUsageFlagBits::eUniformBuffer;
        if (usage & BufferUsage::StorageBuffer) out |= vk::BufferUsageFlagBits::eStorageBuffer;
        if (usage & BufferUsage::IndexBuffer) out |= vk::BufferUsageFlagBits::eIndexBuffer;
        if (usage & BufferUsage::VertexBuffer) out |= vk::BufferUsageFlagBits::eVertexBuffer;
        if (usage & BufferUsage::IndirectBuffer) out |= vk::BufferUsageFlagBits::eIndirectBuffer;
        if (usage & BufferUsage::ShaderDeviceAddress) out |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
        if (usage & BufferUsage::VideoDecodeSrc) out |= vk::BufferUsageFlagBits::eVideoDecodeSrcKHR;
        if (usage & BufferUsage::VideoDecodeDst) out |= vk::BufferUsageFlagBits::eVideoDecodeDstKHR;
        return out;
    }

    constexpr vk::ImageUsageFlags ToVulkanTextureUsage(const Flags<TextureUsage>& usage)
    {
        vk::ImageUsageFlags out{};
        if (usage & TextureUsage::TransferSrc) out |= vk::ImageUsageFlagBits::eTransferSrc;
        if (usage & TextureUsage::TransferDst) out |= vk::ImageUsageFlagBits::eTransferDst;
        if (usage & TextureUsage::Sampled) out |= vk::ImageUsageFlagBits::eSampled;
        if (usage & TextureUsage::Storage) out |= vk::ImageUsageFlagBits::eStorage;
        if (usage & TextureUsage::ColorAttachment) out |= vk::ImageUsageFlagBits::eColorAttachment;
        if (usage & TextureUsage::DepthStencilAttachment) out |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        if (usage & TextureUsage::TransientAttachment) out |= vk::ImageUsageFlagBits::eTransientAttachment;
        if (usage & TextureUsage::InputAttachment) out |= vk::ImageUsageFlagBits::eInputAttachment;
        if (usage & TextureUsage::HostTransfer) out |= vk::ImageUsageFlagBits::eHostTransfer;
        if (usage & TextureUsage::VideoEncodeSrc) out |= vk::ImageUsageFlagBits::eVideoEncodeSrcKHR;
        if (usage & TextureUsage::VideoEncodeDst) out |= vk::ImageUsageFlagBits::eVideoEncodeDstKHR;
        if (usage & TextureUsage::VideoDecodeSrc) out |= vk::ImageUsageFlagBits::eVideoDecodeSrcKHR;
        if (usage & TextureUsage::VideoDecodeDst) out |= vk::ImageUsageFlagBits::eVideoDecodeDstKHR;
        return out;
    }

    constexpr vk::MemoryPropertyFlags ToVulkanMemoryProperties(const Flags<MemoryProperties>& props)
    {
        vk::MemoryPropertyFlags out{};
        if (props & MemoryProperties::DeviceLocal) out |= vk::MemoryPropertyFlagBits::eDeviceLocal;
        if (props & MemoryProperties::HostVisible) out |= vk::MemoryPropertyFlagBits::eHostVisible;
        if (props & MemoryProperties::HostCoherent) out |= vk::MemoryPropertyFlagBits::eHostCoherent;
        if (props & MemoryProperties::HostCached) out |= vk::MemoryPropertyFlagBits::eHostCached;
        if (props & MemoryProperties::LazilyAllocated) out |= vk::MemoryPropertyFlagBits::eLazilyAllocated;
        if (props & MemoryProperties::Protected) out |= vk::MemoryPropertyFlagBits::eProtected;
        return out;
    }

    constexpr vk::SharingMode ToVulkanSharingMode(const SharingMode& mode)
    {
        return mode == SharingMode::Concurrent ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
    }

    constexpr vk::ImageType ToVulkanImageType(const TextureType& type)
    {
        switch (type)
        {
            case TextureType::OneDimensional: return vk::ImageType::e1D;
            case TextureType::ThreeDimensional: return vk::ImageType::e3D;
            case TextureType::TwoDimensional:
            default: return vk::ImageType::e2D;
        }
    }

    constexpr vk::ImageViewType ToVulkanImageViewType(const TextureType& type)
    {
        switch (type)
        {
            case TextureType::OneDimensional: return vk::ImageViewType::e1D;
            case TextureType::ThreeDimensional: return vk::ImageViewType::e3D;
            case TextureType::TwoDimensional:
            default: return vk::ImageViewType::e2D;
        }
    }

    constexpr vk::ImageTiling ToVulkanImageTiling(const TextureTiling& tiling)
    {
        return tiling == TextureTiling::Linear ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal;
    }

    constexpr vk::ImageLayout ToVulkanImageLayout(const TextureLayout& layout)
    {
        switch (layout)
        {
            case TextureLayout::General: return vk::ImageLayout::eGeneral;
            case TextureLayout::ColorAttachment: return vk::ImageLayout::eColorAttachmentOptimal;
            case TextureLayout::DepthStencilAttachment: return vk::ImageLayout::eDepthStencilAttachmentOptimal;
            case TextureLayout::DepthStencilReadOnly: return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            case TextureLayout::DepthReadOnlyStencilAttachment: return vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal;
            case TextureLayout::DepthAttachmentStencilReadOnly: return vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
            case TextureLayout::DepthAttachment: return vk::ImageLayout::eDepthAttachmentOptimal;
            case TextureLayout::DepthReadOnly: return vk::ImageLayout::eDepthReadOnlyOptimal;
            case TextureLayout::StencilAttachment: return vk::ImageLayout::eStencilAttachmentOptimal;
            case TextureLayout::StencilReadOnly: return vk::ImageLayout::eStencilReadOnlyOptimal;
            case TextureLayout::ShaderReadOnly: return vk::ImageLayout::eShaderReadOnlyOptimal;
            case TextureLayout::TransferSrc: return vk::ImageLayout::eTransferSrcOptimal;
            case TextureLayout::TransferDst: return vk::ImageLayout::eTransferDstOptimal;
            case TextureLayout::PreInitialized: return vk::ImageLayout::ePreinitialized;
            case TextureLayout::ReadOnly: return vk::ImageLayout::eReadOnlyOptimal;
            case TextureLayout::Attachment: return vk::ImageLayout::eAttachmentOptimal;
            case TextureLayout::RenderingLocalRead: return vk::ImageLayout::eRenderingLocalRead;
            case TextureLayout::VideoEncodeSrc: return vk::ImageLayout::eVideoEncodeSrcKHR;
            case TextureLayout::VideoEncodeDst: return vk::ImageLayout::eVideoEncodeDstKHR;
            case TextureLayout::VideoDecodeSrc: return vk::ImageLayout::eVideoDecodeSrcKHR;
            case TextureLayout::VideoDecodeDst: return vk::ImageLayout::eVideoDecodeDstKHR;
            case TextureLayout::PresentSrc: return vk::ImageLayout::ePresentSrcKHR;
            case TextureLayout::SharedPresent: return vk::ImageLayout::eSharedPresentKHR;
            case TextureLayout::Undefined:
            default: return vk::ImageLayout::eUndefined;
        }
    }

    constexpr vk::SampleCountFlagBits ToVulkanSampleCount(const SampleCount& samples)
    {
        switch (samples)
        {
            case SampleCount::Sample2: return vk::SampleCountFlagBits::e2;
            case SampleCount::Sample4: return vk::SampleCountFlagBits::e4;
            case SampleCount::Sample8: return vk::SampleCountFlagBits::e8;
            case SampleCount::Sample16: return vk::SampleCountFlagBits::e16;
            case SampleCount::Sample32: return vk::SampleCountFlagBits::e32;
            case SampleCount::Sample64: return vk::SampleCountFlagBits::e64;
            case SampleCount::Sample1:
            default: return vk::SampleCountFlagBits::e1;
        }
    }

    constexpr vk::PrimitiveTopology ToVulkanPrimitiveTopology(const PrimitiveTopology& topology)
    {
        switch (topology)
        {
            case PrimitiveTopology::LineList: return vk::PrimitiveTopology::eLineList;
            case PrimitiveTopology::LineStrip: return vk::PrimitiveTopology::eLineStrip;
            case PrimitiveTopology::TriangleList: return vk::PrimitiveTopology::eTriangleList;
            case PrimitiveTopology::TriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
            case PrimitiveTopology::TriangleFan: return vk::PrimitiveTopology::eTriangleFan;
            case PrimitiveTopology::PointList:
            default: return vk::PrimitiveTopology::ePointList;
        }
    }

    constexpr vk::PolygonMode ToVulkanPolygonMode(const PolygonMode& mode)
    {
        switch (mode)
        {
            case PolygonMode::Line: return vk::PolygonMode::eLine;
            case PolygonMode::Point: return vk::PolygonMode::ePoint;
            case PolygonMode::Fill:
            default: return vk::PolygonMode::eFill;
        }
    }

    constexpr vk::CullModeFlags ToVulkanCullMode(const Flags<CullMode>& mode)
    {
        vk::CullModeFlags out{};
        if (mode & CullMode::Front) out |= vk::CullModeFlagBits::eFront;
        if (mode & CullMode::Back) out |= vk::CullModeFlagBits::eBack;
        return out;
    }

    constexpr vk::FrontFace ToVulkanFrontFace(const FrontFace& face)
    {
        return face == FrontFace::CounterClockwise ? vk::FrontFace::eCounterClockwise : vk::FrontFace::eClockwise;
    }

    constexpr vk::BlendFactor ToVulkanBlendFactor(const BlendFactor& factor)
    {
        switch (factor)
        {
            case BlendFactor::One: return vk::BlendFactor::eOne;
            case BlendFactor::SrcColor: return vk::BlendFactor::eSrcColor;
            case BlendFactor::OneMinusSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
            case BlendFactor::DstColor: return vk::BlendFactor::eDstColor;
            case BlendFactor::OneMinusDstColor: return vk::BlendFactor::eOneMinusDstColor;
            case BlendFactor::SrcAlpha: return vk::BlendFactor::eSrcAlpha;
            case BlendFactor::OneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
            case BlendFactor::DstAlpha: return vk::BlendFactor::eDstAlpha;
            case BlendFactor::OneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
            case BlendFactor::ConstantColor: return vk::BlendFactor::eConstantColor;
            case BlendFactor::OneMinusConstantColor: return vk::BlendFactor::eOneMinusConstantColor;
            case BlendFactor::ConstantAlpha: return vk::BlendFactor::eConstantAlpha;
            case BlendFactor::OneMinusConstantAlpha: return vk::BlendFactor::eOneMinusConstantAlpha;
            case BlendFactor::Zero:
            default: return vk::BlendFactor::eZero;
        }
    }

    constexpr vk::BlendOp ToVulkanBlendOp(const BlendOp& op)
    {
        switch (op)
        {
            case BlendOp::Subtract: return vk::BlendOp::eSubtract;
            case BlendOp::SubtractReverse: return vk::BlendOp::eReverseSubtract;
            case BlendOp::Min: return vk::BlendOp::eMin;
            case BlendOp::Max: return vk::BlendOp::eMax;
            case BlendOp::Add:
            default: return vk::BlendOp::eAdd;
        }
    }

    constexpr vk::ColorComponentFlags ToVulkanColorComponents(const Flags<ColorComponents>& mask)
    {
        vk::ColorComponentFlags out{};
        if (mask & ColorComponents::R) out |= vk::ColorComponentFlagBits::eR;
        if (mask & ColorComponents::G) out |= vk::ColorComponentFlagBits::eG;
        if (mask & ColorComponents::B) out |= vk::ColorComponentFlagBits::eB;
        if (mask & ColorComponents::A) out |= vk::ColorComponentFlagBits::eA;
        return out;
    }

    constexpr vk::LogicOp ToVulkanLogicOp(const LogicOp& op)
    {
        switch (op)
        {
            case LogicOp::And: return vk::LogicOp::eAnd;
            case LogicOp::AndReversed: return vk::LogicOp::eAndReverse;
            case LogicOp::AndInverted: return vk::LogicOp::eAndInverted;
            case LogicOp::Copy: return vk::LogicOp::eCopy;
            case LogicOp::CopyInverted: return vk::LogicOp::eCopyInverted;
            case LogicOp::NoOp: return vk::LogicOp::eNoOp;
            case LogicOp::XOR: return vk::LogicOp::eXor;
            case LogicOp::OR: return vk::LogicOp::eOr;
            case LogicOp::ORReverse: return vk::LogicOp::eOrReverse;
            case LogicOp::ORInverted: return vk::LogicOp::eOrInverted;
            case LogicOp::NOR: return vk::LogicOp::eNor;
            case LogicOp::NAND: return vk::LogicOp::eNand;
            case LogicOp::Equivalent: return vk::LogicOp::eEquivalent;
            case LogicOp::Invert: return vk::LogicOp::eInvert;
            case LogicOp::Set: return vk::LogicOp::eSet;
            case LogicOp::Clear:
            default: return vk::LogicOp::eClear;
        }
    }

    // Single-bit ShaderStage -> Vulkan stage flag bit (for pipeline stage create info / shaderc kind lookup)
    constexpr vk::ShaderStageFlagBits ToVulkanShaderStage(const ShaderStage& stage)
    {
        switch (stage)
        {
            case ShaderStage::TessellationControl: return vk::ShaderStageFlagBits::eTessellationControl;
            case ShaderStage::TessellationEval: return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case ShaderStage::Geometry: return vk::ShaderStageFlagBits::eGeometry;
            case ShaderStage::Fragment: return vk::ShaderStageFlagBits::eFragment;
            case ShaderStage::Task: return vk::ShaderStageFlagBits::eTaskEXT;
            case ShaderStage::Mesh: return vk::ShaderStageFlagBits::eMeshEXT;
            case ShaderStage::Vertex:
            default: return vk::ShaderStageFlagBits::eVertex;
        }
    }

    constexpr vk::ShaderStageFlags ToVulkanShaderStageFlags(const Flags<ShaderStage>& stages)
    {
        vk::ShaderStageFlags out{};
        if (stages & ShaderStage::Vertex) out |= vk::ShaderStageFlagBits::eVertex;
        if (stages & ShaderStage::TessellationControl) out |= vk::ShaderStageFlagBits::eTessellationControl;
        if (stages & ShaderStage::TessellationEval) out |= vk::ShaderStageFlagBits::eTessellationEvaluation;
        if (stages & ShaderStage::Geometry) out |= vk::ShaderStageFlagBits::eGeometry;
        if (stages & ShaderStage::Fragment) out |= vk::ShaderStageFlagBits::eFragment;
        if (stages & ShaderStage::Task) out |= vk::ShaderStageFlagBits::eTaskEXT;
        if (stages & ShaderStage::Mesh) out |= vk::ShaderStageFlagBits::eMeshEXT;
        return out;
    }

    constexpr vk::IndexType ToVulkanIndexType(const IndexType& type)
    {
        switch (type)
        {
            case IndexType::Uint8: return vk::IndexType::eUint8KHR;
            case IndexType::Uint32: return vk::IndexType::eUint32;
            case IndexType::Uint16:
            default: return vk::IndexType::eUint16;
        }
    }

    // Resolves the concrete Vulkan descriptor type for a resource-group binding. GPUResourceType's
    // explicit buffer/texture/sampler subtypes map 1:1; the generic Buffer/Texture/Sampler entries
    // fall back to the most common concrete usage (Uniform Buffer / Sampled Texture / Combined Sampler).
    constexpr vk::DescriptorType ToVulkanDescriptorType(const GPUResourceType& type)
    {
        switch (type)
        {
            case GPUResourceType::StorageBuffer: return vk::DescriptorType::eStorageBuffer;
            case GPUResourceType::Buffer:
            case GPUResourceType::UniformBuffer: return vk::DescriptorType::eUniformBuffer;
            case GPUResourceType::StorageBufferDynamic: return vk::DescriptorType::eStorageBufferDynamic;
            case GPUResourceType::UniformBufferDynamic: return vk::DescriptorType::eUniformBufferDynamic;
            case GPUResourceType::StorageTexelBuffer: return vk::DescriptorType::eStorageTexelBuffer;
            case GPUResourceType::UniformTexelBuffer: return vk::DescriptorType::eUniformTexelBuffer;
            case GPUResourceType::StorageTexture: return vk::DescriptorType::eStorageImage;
            case GPUResourceType::Texture:
            case GPUResourceType::SampledTexture: return vk::DescriptorType::eSampledImage;
            case GPUResourceType::Sampler: return vk::DescriptorType::eSampler;
            case GPUResourceType::CombinedTextureSampler:
            default: return vk::DescriptorType::eCombinedImageSampler;
        }
    }

    constexpr vk::ColorSpaceKHR ToVulkanColorSpace(const ColorSpace& space)
    {
        return vk::ColorSpaceKHR::eSrgbNonlinear;
    }

    constexpr vk::CompositeAlphaFlagBitsKHR ToVulkanCompositeAlpha(const CompositeAlpha& alpha)
    {
        switch (alpha)
        {
            case CompositeAlpha::PreMultiplied: return vk::CompositeAlphaFlagBitsKHR::ePreMultiplied;
            case CompositeAlpha::PostMultiplied: return vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
            case CompositeAlpha::Inherit: return vk::CompositeAlphaFlagBitsKHR::eInherit;
            case CompositeAlpha::Opaque:
            default: return vk::CompositeAlphaFlagBitsKHR::eOpaque;
        }
    }

    constexpr vk::PresentModeKHR ToVulkanPresentMode(const PresentMode& mode)
    {
        switch (mode)
        {
            case PresentMode::Immediate: return vk::PresentModeKHR::eImmediate;
            case PresentMode::DoubleBufferedVSync: return vk::PresentModeKHR::eFifo;
            case PresentMode::TripleBufferedVSync: return vk::PresentModeKHR::eFifo;
            case PresentMode::QuadBufferedVSync: return vk::PresentModeKHR::eFifo;
            case PresentMode::DoubleBufferedLowLatency: return vk::PresentModeKHR::eMailbox;
            case PresentMode::TripleBufferedLowLatency: return vk::PresentModeKHR::eMailbox;
            case PresentMode::QuadBufferedLowLatency: return vk::PresentModeKHR::eMailbox;
            default: return vk::PresentModeKHR::eFifo;
        }
    }

    constexpr vk::ImageAspectFlags ToVulkanImageAspect(const Flags<ImageAspect>& aspect)
    {
        vk::ImageAspectFlags out{};
        if (aspect & ImageAspect::Color) out |= vk::ImageAspectFlagBits::eColor;
        if (aspect & ImageAspect::Depth) out |= vk::ImageAspectFlagBits::eDepth;
        if (aspect & ImageAspect::Stencil) out |= vk::ImageAspectFlagBits::eStencil;
        if (aspect & ImageAspect::Metadata) out |= vk::ImageAspectFlagBits::eMetadata;
        if (aspect & ImageAspect::Plane0) out |= vk::ImageAspectFlagBits::ePlane0;
        if (aspect & ImageAspect::Plane1) out |= vk::ImageAspectFlagBits::ePlane1;
        if (aspect & ImageAspect::Plane2) out |= vk::ImageAspectFlagBits::ePlane2;
        return out;
    }

    constexpr vk::ComponentSwizzle ToVulkanSwizzle(const SwizzleComponent& swizzle)
    {
        switch (swizzle)
        {
            case SwizzleComponent::Zero: return vk::ComponentSwizzle::eZero;
            case SwizzleComponent::One: return vk::ComponentSwizzle::eOne;
            case SwizzleComponent::R: return vk::ComponentSwizzle::eR;
            case SwizzleComponent::G: return vk::ComponentSwizzle::eG;
            case SwizzleComponent::B: return vk::ComponentSwizzle::eB;
            case SwizzleComponent::A: return vk::ComponentSwizzle::eA;
            case SwizzleComponent::Identity:
            default: return vk::ComponentSwizzle::eIdentity;
        }
    }

    constexpr vk::Filter ToVulkanFilter(const Filter& filter)
    {
        switch (filter)
        {
            case Filter::Nearest: return vk::Filter::eNearest;
            case Filter::Linear: return vk::Filter::eLinear;
            case Filter::Cubic: return vk::Filter::eCubicIMG;
            default: return vk::Filter::eNearest;
        }
    }

    constexpr vk::SamplerMipmapMode ToVulkanMipmapMode(const SamplerMipmapMode& mode)
    {
        switch (mode)
        {
            case SamplerMipmapMode::Nearest: return vk::SamplerMipmapMode::eNearest;
            case SamplerMipmapMode::Linear: return vk::SamplerMipmapMode::eLinear;
            default: return vk::SamplerMipmapMode::eNearest;
        }
    }

    constexpr vk::SamplerAddressMode ToVulkanAddressMode(const SamplerAddressMode& mode)
    {
        switch (mode)
        {
            case SamplerAddressMode::Repeat: return vk::SamplerAddressMode::eRepeat;
            case SamplerAddressMode::RepeatMirrored: return vk::SamplerAddressMode::eMirroredRepeat;
            case SamplerAddressMode::ClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
            case SamplerAddressMode::ClampToBorder: return vk::SamplerAddressMode::eClampToBorder;
            case SamplerAddressMode::ClampToEdgeMirrored: return vk::SamplerAddressMode::eMirrorClampToEdge;
            default: return vk::SamplerAddressMode::eRepeat;
        }
    }

    constexpr vk::BorderColor ToVulkanBorderColor(const BorderColor& color)
    {
        switch (color)
        {
            case BorderColor::TransparentBlackInt: return vk::BorderColor::eIntTransparentBlack;
            case BorderColor::TransparentBlackFloat: return vk::BorderColor::eFloatTransparentBlack;
            case BorderColor::OpaqueBlackInt: return vk::BorderColor::eIntOpaqueBlack;
            case BorderColor::OpaqueBlackFloat: return vk::BorderColor::eFloatOpaqueBlack;
            case BorderColor::OpaqueWhiteInt: return vk::BorderColor::eIntOpaqueWhite;
            case BorderColor::OpaqueWhiteFloat: return vk::BorderColor::eFloatOpaqueWhite;
            default: return vk::BorderColor::eIntOpaqueBlack;
        }
    }

    constexpr vk::PipelineBindPoint ToVulkanPipelineBindPoint(const PipelineBindPoint& bind_point)
    {
        switch (bind_point)
        {
            case PipelineBindPoint::Compute: return vk::PipelineBindPoint::eCompute;
            case PipelineBindPoint::RayTracing: return vk::PipelineBindPoint::eRayTracingKHR;
            case PipelineBindPoint::Graphics:
            default: return vk::PipelineBindPoint::eGraphics;
        }
    }

    constexpr vk::AccessFlags2 ToVulkanAccessFlags2(const Flags<PipelineAccess>& access)
    {
        vk::AccessFlags2 out{};
        if (access & PipelineAccess::IndirectCommandRead) out |= vk::AccessFlagBits2::eIndirectCommandRead;
        if (access & PipelineAccess::IndexRead) out |= vk::AccessFlagBits2::eIndexRead;
        if (access & PipelineAccess::VertexAttributeRead) out |= vk::AccessFlagBits2::eVertexAttributeRead;
        if (access & PipelineAccess::UniformRead) out |= vk::AccessFlagBits2::eUniformRead;
        if (access & PipelineAccess::InputAttachmentRead) out |= vk::AccessFlagBits2::eInputAttachmentRead;
        if (access & PipelineAccess::ShaderRead) out |= vk::AccessFlagBits2::eShaderRead;
        if (access & PipelineAccess::ShaderWrite) out |= vk::AccessFlagBits2::eShaderWrite;
        if (access & PipelineAccess::ColorAttachmentRead) out |= vk::AccessFlagBits2::eColorAttachmentRead;
        if (access & PipelineAccess::ColorAttachmentWrite) out |= vk::AccessFlagBits2::eColorAttachmentWrite;
        if (access & PipelineAccess::DepthStencilAttachmentRead) out |= vk::AccessFlagBits2::eDepthStencilAttachmentRead;
        if (access & PipelineAccess::DepthStencilAttachmentWrite) out |= vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
        if (access & PipelineAccess::TransferRead) out |= vk::AccessFlagBits2::eTransferRead;
        if (access & PipelineAccess::TransferWrite) out |= vk::AccessFlagBits2::eTransferWrite;
        if (access & PipelineAccess::HostRead) out |= vk::AccessFlagBits2::eHostRead;
        if (access & PipelineAccess::HostWrite) out |= vk::AccessFlagBits2::eHostWrite;
        if (access & PipelineAccess::MemoryRead) out |= vk::AccessFlagBits2::eMemoryRead;
        if (access & PipelineAccess::MemoryWrite) out |= vk::AccessFlagBits2::eMemoryWrite;
        if (access & PipelineAccess::ShaderSampledRead) out |= vk::AccessFlagBits2::eShaderSampledRead;
        // Vulkan has no "sampled write" access - sampled images are read-only. Fall back to the
        //  generic shader-write bit so a caller that (incorrectly) requests it still gets a safe barrier.
        if (access & PipelineAccess::ShaderSampledWrite) out |= vk::AccessFlagBits2::eShaderWrite;
        if (access & PipelineAccess::ShaderStorageRead) out |= vk::AccessFlagBits2::eShaderStorageRead;
        if (access & PipelineAccess::ShaderStorageWrite) out |= vk::AccessFlagBits2::eShaderStorageWrite;
        if (access & PipelineAccess::VideoEncodeRead) out |= vk::AccessFlagBits2::eVideoEncodeReadKHR;
        if (access & PipelineAccess::VideoEncodeWrite) out |= vk::AccessFlagBits2::eVideoEncodeWriteKHR;
        if (access & PipelineAccess::VideoDecodeRead) out |= vk::AccessFlagBits2::eVideoDecodeReadKHR;
        if (access & PipelineAccess::VideoDecodeWrite) out |= vk::AccessFlagBits2::eVideoDecodeWriteKHR;
        return out;
    }

    constexpr vk::PipelineStageFlags2 ToVulkanPipelineStage2(const PipelineStage& stage)
    {
        switch (stage)
        {
            case PipelineStage::TopOfPipe: return vk::PipelineStageFlagBits2::eTopOfPipe;
            case PipelineStage::DrawIndirect: return vk::PipelineStageFlagBits2::eDrawIndirect;
            case PipelineStage::VertexInput: return vk::PipelineStageFlagBits2::eVertexInput;
            case PipelineStage::VertexShader: return vk::PipelineStageFlagBits2::eVertexShader;
            case PipelineStage::TessellationControlShader: return vk::PipelineStageFlagBits2::eTessellationControlShader;
            case PipelineStage::TessellationEvalShader: return vk::PipelineStageFlagBits2::eTessellationEvaluationShader;
            case PipelineStage::GeometryShader: return vk::PipelineStageFlagBits2::eGeometryShader;
            case PipelineStage::FragmentShader: return vk::PipelineStageFlagBits2::eFragmentShader;
            case PipelineStage::FragmentTestEarly: return vk::PipelineStageFlagBits2::eEarlyFragmentTests;
            case PipelineStage::FragmentTestLate: return vk::PipelineStageFlagBits2::eLateFragmentTests;
            case PipelineStage::ColorAttachmentOutput: return vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            case PipelineStage::ComputeShader: return vk::PipelineStageFlagBits2::eComputeShader;
            case PipelineStage::AllTransfer: return vk::PipelineStageFlagBits2::eAllTransfer;
            case PipelineStage::Transfer: return vk::PipelineStageFlagBits2::eTransfer;
            case PipelineStage::BottomOfPipe: return vk::PipelineStageFlagBits2::eBottomOfPipe;
            case PipelineStage::Host: return vk::PipelineStageFlagBits2::eHost;
            case PipelineStage::AllGraphics: return vk::PipelineStageFlagBits2::eAllGraphics;
            case PipelineStage::ALlCommands: return vk::PipelineStageFlagBits2::eAllCommands;
            case PipelineStage::Copy: return vk::PipelineStageFlagBits2::eCopy;
            case PipelineStage::Resolve: return vk::PipelineStageFlagBits2::eResolve;
            case PipelineStage::Blit: return vk::PipelineStageFlagBits2::eBlit;
            case PipelineStage::Clear: return vk::PipelineStageFlagBits2::eClear;
            case PipelineStage::IndexInput: return vk::PipelineStageFlagBits2::eIndexInput;
            case PipelineStage::VertexAttributeInput: return vk::PipelineStageFlagBits2::eVertexAttributeInput;
            case PipelineStage::PreRasterShaders: return vk::PipelineStageFlagBits2::ePreRasterizationShaders;
            case PipelineStage::VideoEncode: return vk::PipelineStageFlagBits2::eVideoEncodeKHR;
            case PipelineStage::VideoDecode: return vk::PipelineStageFlagBits2::eVideoDecodeKHR;
            case PipelineStage::RayTracingShader: return vk::PipelineStageFlagBits2::eRayTracingShaderKHR;
            case PipelineStage::TaskShader: return vk::PipelineStageFlagBits2::eTaskShaderEXT;
            case PipelineStage::MeshShader: return vk::PipelineStageFlagBits2::eMeshShaderEXT;
            case PipelineStage::None:
            default: return vk::PipelineStageFlagBits2::eNone;
        }
    }

    constexpr vk::CompareOp ToVulkanCompareOp(const CompareOp& op)
    {
        switch (op)
        {
            case CompareOp::Never: return vk::CompareOp::eNever;
            case CompareOp::Less: return vk::CompareOp::eLess;
            case CompareOp::Equal: return vk::CompareOp::eEqual;
            case CompareOp::LessOrEqual: return vk::CompareOp::eLessOrEqual;
            case CompareOp::Greater: return vk::CompareOp::eGreater;
            case CompareOp::GreaterOrEqual: return vk::CompareOp::eGreaterOrEqual;
            case CompareOp::NotEqual: return vk::CompareOp::eNotEqual;
            case CompareOp::Always: return vk::CompareOp::eAlways;
            default: return vk::CompareOp::eNever;
        }
    }

    constexpr std::array<f32, 4> ToDebugColor(const u32& rgba)
    {
        return {
            static_cast<f32>((rgba >> 24) & 0xFF) / 255.0f,
            static_cast<f32>((rgba >> 16) & 0xFF) / 255.0f,
            static_cast<f32>((rgba >> 8) & 0xFF) / 255.0f,
            static_cast<f32>(rgba & 0xFF) / 255.0f,
        };
    }
}
