module;

#include <AurionLog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.hpp>

#ifdef AURION_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elifdef AURION_PLATFORM_LINUX
#include <fcntl.h>
#endif

#include <bit>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <ranges>
#include <algorithm>
#include <string>
#include <limits>

module Aurion.Vulkan;

import Aurion.FileSystem;
import Aurion.Types;

namespace Aurion::Vulkan
{
    Driver::Driver(const API* vulkan_api, const PhysicalDeviceProperties& pDevice_props, const DeviceProperties& device_props)
        : m_vulkan_api(nullptr), m_physical_device(nullptr), m_logical_device(nullptr)
    {
        if (!vulkan_api)
            throw std::runtime_error("[Vulkan Driver] Invalid Vulkan API Instance");

        m_vulkan_api = vulkan_api;
        m_physical_device = m_vulkan_api->GetPhysicalDevice(pDevice_props);

        // Create the logical device for interfacing with the physical device
        // ------------------------------------------------------------------

        // Get all physical device queue descriptions
        std::vector<vk::QueueFamilyProperties> device_qfp = m_physical_device.getQueueFamilyProperties();

        // Track which queue descriptions belong to which queue family
        std::unordered_map<u32, std::vector<QueueDescription>> qf_descriptions{};
        std::unordered_map<u32, QueueDescription> qf_descs_flattened{};
        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};

        // Alias requested device queues
        {
            // Aggregate queue descriptions into their most optimal queue family
            for (const auto& desc : device_props.queues)
            {
                // Filter for all queue families that match the queue description flags
                auto matches = device_qfp | std::views::filter(
                    [&](const auto& prop)
                    {
                        return (prop.queueFlags & desc.flags) != static_cast<vk::QueueFlags>(0);
                    }
                );

                // If no matching queue families were found, queue creation shouldn't occur
                if (matches.empty())
                    throw std::runtime_error("[Vulkan Driver] No queue family matches the requested queue flags.");

                // Find the best-fit match for this queue description
                u32 best_bit_count = UINT32_MAX;
                u64 best_index = 0; // Default to first element
                for (const auto& qfp : matches)
                {
                    // Count the number of bit flags available set on this queue family
                    const u32 bit_count = std::popcount(static_cast<VkQueueFlags>(qfp.queueFlags));

                    // TODO: Allow for shared queue usage (largest subset)

                    // Choose the queue with the least bit flags (smallest subset)
                    if (bit_count < best_bit_count)
                    {
                        best_bit_count = bit_count;
                        auto it = std::ranges::find(device_qfp, qfp);
                        best_index = std::distance(device_qfp.begin(), it);
                    }
                }

                // Create an entry in the description map for this queue description
                if (!qf_descriptions.contains(best_index))
                    qf_descriptions[best_index] = { desc };
                else
                    qf_descriptions[best_index].push_back(desc);
            }

            // Once queue descriptions have been aggregated by queue family, flatten all descriptions into
            //  one queue description per queue family
            for (const auto& [index, desc_arr] : qf_descriptions)
            {
                // Assign a new aggregate queue description
                qf_descs_flattened[index] = {
                    .count = 0,
                    .priorities = {}
                };

                // For each unique queue family
                for (const auto qf_desc : desc_arr)
                {
                    // Increase the number of queues of this family to create
                    qf_descs_flattened[index].count += qf_desc.count;

                    // Append all queue priorities
                    qf_descs_flattened[index].priorities.append_range(qf_desc.priorities);
                }
            }

            // After flattening, generate DeviceQueueCreateInfo structures
            for (const auto& [index, desc] : qf_descs_flattened)
            {
                AURION_WARN("Queue Family Index [%d]: Creating %d queues.", index, desc.count);

                vk::DeviceQueueCreateInfo cInfo{};
                cInfo.queueFamilyIndex = static_cast<u32>(index);
                cInfo.queueCount = static_cast<u32>(desc.count);
                cInfo.pQueuePriorities = desc.priorities.data();

                queue_create_infos.push_back(cInfo);
            }
        }

        // Then, generate the logical device
        vk::DeviceCreateInfo dcInfo{};
        dcInfo.pNext = device_props.features;
        dcInfo.queueCreateInfoCount = static_cast<u32>(queue_create_infos.size());
        dcInfo.pQueueCreateInfos = queue_create_infos.data();
        dcInfo.enabledExtensionCount = static_cast<u32>(device_props.extensions.size());
        dcInfo.ppEnabledExtensionNames = device_props.extensions.data();

        m_logical_device = vk::raii::Device(m_physical_device, dcInfo);

        // Allocate a command pool and command buffers for each queue in each queue family
        for (const auto& [index, desc] : qf_descs_flattened)
        {
            auto& qf_props = device_qfp[index];

            // Create a new entry in the queue family map
            m_queue_families[index] = QueueFamily{};
            auto& qf = m_queue_families[index];

            // Copy over queue family data
            qf.index = static_cast<u32>(index);
            qf.minImageTransferGranularity = qf_props.minImageTransferGranularity;
            qf.queueCount = static_cast<u32>(desc.count);
            qf.queueFlags = qf_props.queueFlags;
            qf.timestampValidBits = qf_props.timestampValidBits;

            // Create a command pool for this queue family to allocate from
            qf.GenerateCommandPool(m_logical_device, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

            // NOTE: Command buffer allocations can be forwarded to RenderContext creation, where command buffers
            //          are required.
        }

        // Per-frame fences are the responsibility of a RenderContext
    }

    Driver::~Driver()
    {
        // Wait for any GPU work to finish before tearing down owned resources
        m_logical_device.waitIdle();
    }

    // --- Single Resource Allocation ---

    SurfaceHandle Driver::CreateSurface(const SurfaceDescription& desc)
    {
        SurfaceData& data = m_surfaces.emplace_back(nullptr, desc);
        data.surface = CreateWindowSurface(desc.window);

        SurfaceHandle handle;
        handle.value = MakeGPUHandleValue(GPUResourceType::Surface, m_surfaces.size() - 1, 0);
        return handle;
    }

    PipelineHandle Driver::CreatePipeline(const PipelineDescription& desc)
    {
        // BIG TODO:
        //  1. Differentiate between pipeline types (Graphics, Compute, RayTracing)
        //  2. Some applications won't support dynamic rendering. Legacy support might be required.


        // GraphicsPipelineDescription declares its own `pType` (defaulting to Graphics) that shadows
        // PipelineDescription::pType rather than overriding it - reading `desc.pType` through this
        // base reference would see the base's Unknown default, not the derived one. Only
        // GraphicsPipelineDescription exists today, so downcast unconditionally; this becomes an
        // actual dispatch once Compute/RayTrace descriptions exist.
        const auto& gfx_desc = static_cast<const GraphicsPipelineDescription&>(desc);

        // Shader stages + descriptor set layouts (one set per shader, in shaders-list order)
        std::vector<vk::PipelineShaderStageCreateInfo> stage_infos{};
        std::vector<vk::DescriptorSetLayout> set_layouts{};
        set_layouts.reserve(gfx_desc.shaders.size());

        for (const auto& shader_handle : gfx_desc.shaders)
        {
            if (shader_handle.value == 0)
                throw std::runtime_error("[VulkanDriver::CreatePipeline] Invalid shader handle.");

            const ShaderData& shader_data = m_shaders[GPUHandleIndex(shader_handle)];

            for (const auto& entry : shader_data.entry_points)
            {
                const auto it = shader_data.modules.find(entry.stage);
                if (it == shader_data.modules.end()) continue;

                vk::PipelineShaderStageCreateInfo stage_info{};
                stage_info.stage = ToVulkanShaderStage(entry.stage);
                stage_info.module = *it->second;
                stage_info.pName = entry.id.c_str();
                stage_infos.push_back(stage_info);
            }

            if (shader_data.group_layout.value == 0)
                continue;

            const ResourceGroupLayoutData& layout_data = m_resource_group_layouts[GPUHandleIndex(shader_data.group_layout)];
            set_layouts.push_back(*layout_data.layout);
        }

        // Vertex input
        std::vector<vk::VertexInputBindingDescription> vertex_bindings{};
        for (const auto& b : gfx_desc.vertex_input.bindings)
            vertex_bindings.push_back({ b.binding, static_cast<u32>(b.stride),
                b.input_rate == VertexInputRate::Instance ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex });

        std::vector<vk::VertexInputAttributeDescription> vertex_attributes{};
        for (const auto& a : gfx_desc.vertex_input.attributes)
            vertex_attributes.push_back({ a.location, a.binding, ToVulkanFormat(a.format), a.offset });

        vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.vertexBindingDescriptionCount = static_cast<u32>(vertex_bindings.size());
        vertex_input_info.pVertexBindingDescriptions = vertex_bindings.data();
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<u32>(vertex_attributes.size());
        vertex_input_info.pVertexAttributeDescriptions = vertex_attributes.data();

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_info{};
        input_assembly_info.topology = ToVulkanPrimitiveTopology(gfx_desc.input_assembly.topology);
        input_assembly_info.primitiveRestartEnable = gfx_desc.input_assembly.enable_primitive_restart;

        vk::PipelineRasterizationStateCreateInfo rasterization_info{};
        rasterization_info.polygonMode = ToVulkanPolygonMode(gfx_desc.rasterization.polygon_mode);
        rasterization_info.cullMode = ToVulkanCullMode(gfx_desc.rasterization.cull_mode);
        rasterization_info.frontFace = ToVulkanFrontFace(gfx_desc.rasterization.front_face);
        rasterization_info.depthBiasConstantFactor = gfx_desc.rasterization.depth_bias_constant;
        rasterization_info.depthBiasClamp = gfx_desc.rasterization.depth_bias_clamp;
        rasterization_info.depthBiasSlopeFactor = gfx_desc.rasterization.depth_bias_slope;
        rasterization_info.lineWidth = gfx_desc.rasterization.line_width;
        rasterization_info.depthClampEnable = gfx_desc.rasterization.enable_depth_clamp;
        rasterization_info.rasterizerDiscardEnable = gfx_desc.rasterization.enable_rasterizer_discard;
        rasterization_info.depthBiasEnable = gfx_desc.rasterization.enable_depth_bias;

        std::vector<vk::Viewport> viewports{};
        for (const auto& v : gfx_desc.viewport.viewports)
            viewports.push_back({ v.x, v.y, v.width, v.height, v.min_depth, v.max_depth });

        std::vector<vk::Rect2D> scissors{};
        for (const auto& s : gfx_desc.viewport.scissors)
            scissors.push_back({ { static_cast<i32>(s.offset_x), static_cast<i32>(s.offset_y) }, { s.width, s.height } });

        vk::PipelineViewportStateCreateInfo viewport_info{};
        viewport_info.viewportCount = static_cast<u32>(std::max<size_t>(viewports.size(), 1));
        viewport_info.pViewports = viewports.empty() ? nullptr : viewports.data();
        viewport_info.scissorCount = static_cast<u32>(std::max<size_t>(scissors.size(), 1));
        viewport_info.pScissors = scissors.empty() ? nullptr : scissors.data();

        vk::PipelineMultisampleStateCreateInfo multisample_info{};
        multisample_info.rasterizationSamples = ToVulkanSampleCount(gfx_desc.multisampling.rasterization_samples);
        multisample_info.sampleShadingEnable = gfx_desc.multisampling.enable_sample_shading;
        multisample_info.alphaToCoverageEnable = gfx_desc.multisampling.enable_alpha_to_coverage;
        multisample_info.alphaToOneEnable = gfx_desc.multisampling.enable_alpha_to_one;
        multisample_info.pSampleMask = gfx_desc.multisampling.sample_mask;

        std::vector<vk::PipelineColorBlendAttachmentState> blend_attachments{};
        for (const auto& attachment : gfx_desc.color_blending.blend_attachments)
        {
            vk::PipelineColorBlendAttachmentState state{};
            state.blendEnable = attachment.enable_blend;
            state.srcColorBlendFactor = ToVulkanBlendFactor(attachment.src_color_blend_factor);
            state.dstColorBlendFactor = ToVulkanBlendFactor(attachment.dst_color_blend_factor);
            state.colorBlendOp = ToVulkanBlendOp(attachment.color_blend_op);
            state.srcAlphaBlendFactor = ToVulkanBlendFactor(attachment.src_alpha_blend_factor);
            state.dstAlphaBlendFactor = ToVulkanBlendFactor(attachment.dst_alpha_blend_factor);
            state.alphaBlendOp = ToVulkanBlendOp(attachment.alpha_blend_op);
            state.colorWriteMask = ToVulkanColorComponents(attachment.color_write_mask);
            blend_attachments.push_back(state);
        }

        vk::PipelineColorBlendStateCreateInfo color_blend_info{};
        color_blend_info.logicOpEnable = gfx_desc.color_blending.enable_logic_op;
        color_blend_info.logicOp = ToVulkanLogicOp(gfx_desc.color_blending.logic_op);
        color_blend_info.attachmentCount = static_cast<u32>(blend_attachments.size());
        color_blend_info.pAttachments = blend_attachments.data();
        for (u32 i = 0; i < 4; ++i)
            color_blend_info.blendConstants[i] = gfx_desc.color_blending.blend_constants[i];

        std::vector<vk::DynamicState> dynamic_states{};
        if (gfx_desc.viewport.enable_dynamic_viewport) dynamic_states.push_back(vk::DynamicState::eViewport);
        if (gfx_desc.viewport.enable_dynamic_scissor) dynamic_states.push_back(vk::DynamicState::eScissor);

        vk::PipelineDynamicStateCreateInfo dynamic_state_info{};
        dynamic_state_info.dynamicStateCount = static_cast<u32>(dynamic_states.size());
        dynamic_state_info.pDynamicStates = dynamic_states.data();

        std::vector<vk::Format> color_formats{};
        for (const auto& f : gfx_desc.color_attachment.formats)
            color_formats.push_back(ToVulkanFormat(f));

        vk::PipelineRenderingCreateInfo rendering_info{};
        rendering_info.viewMask = gfx_desc.color_attachment.view_mask;
        rendering_info.colorAttachmentCount = static_cast<u32>(color_formats.size());
        rendering_info.pColorAttachmentFormats = color_formats.data();
        rendering_info.depthAttachmentFormat = ToVulkanFormat(gfx_desc.color_attachment.depth_format);
        rendering_info.stencilAttachmentFormat = ToVulkanFormat(gfx_desc.color_attachment.stencil_format);

        vk::PipelineLayoutCreateInfo layout_info{};
        layout_info.setLayoutCount = static_cast<u32>(set_layouts.size());
        layout_info.pSetLayouts = set_layouts.data();

        PipelineData& data = m_pipelines.emplace_back(nullptr, nullptr);
        data.layout = vk::raii::PipelineLayout(m_logical_device, layout_info);

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.stageCount = static_cast<u32>(stage_infos.size());
        pipeline_info.pStages = stage_infos.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly_info;
        pipeline_info.pViewportState = &viewport_info;
        pipeline_info.pRasterizationState = &rasterization_info;
        pipeline_info.pMultisampleState = &multisample_info;
        pipeline_info.pColorBlendState = &color_blend_info;
        pipeline_info.pDynamicState = &dynamic_state_info;
        pipeline_info.layout = *data.layout;

        if (gfx_desc.enable_dynamic_rendering)
            pipeline_info.pNext = &rendering_info;

        data.pipeline = vk::raii::Pipeline(m_logical_device, nullptr, pipeline_info);

        PipelineHandle handle;
        handle.value = MakeGPUHandleValue(GPUResourceType::Pipeline, m_pipelines.size() - 1, 0);

        return handle;
    }

    ShaderHandle Driver::CreateShader(const ShaderDescription& desc)
    {
        // Read shader source from disk
        FSFile file(desc.path.c_str());

        FSFileOpenParams params{};
#ifdef AURION_PLATFORM_WINDOWS
        params.dwAccess = GENERIC_READ;
        params.dwShareMode = FILE_SHARE_READ;
        params.lpSecurityAttr = NULL;
        params.dwCreateDisposition = OPEN_EXISTING;
        params.dwFlagsAndAttr = FILE_ATTRIBUTE_NORMAL;
        params.hTemplateFile = NULL;
#elifdef AURION_PLATFORM_LINUX
        params.flags = O_RDONLY;
        params.access = 0;
#endif

        file.Open(params);
        if (!file.IsOpen())
            throw std::runtime_error("[Vulkan Driver] CreateShader: failed to open file '" + desc.path + "'.");

        const FSMetadata metadata = file.GetMetadata(true);

        // Clamp buffer size to a multiple of 4 - Vulkan expects SPIR-V-aligned uint32_t code
        const u64 rem = metadata.size % sizeof(u32);
        const u64 clamped_size = rem == 0 ? metadata.size : metadata.size + (sizeof(u32) - rem);

        std::vector<char> buffer(clamped_size);

#ifdef AURION_PLATFORM_WINDOWS
        const u64 whence = FILE_BEGIN;
#elifdef AURION_PLATFORM_LINUX
        const u64 whence = SEEK_SET;
#endif

        if (file.Tell() != 0)
            file.Seek(0, whence);

        file.Read(buffer.data(), metadata.size);
        file.Close();

        ShaderData& data = m_shaders.emplace_back();
        data.entry_points = desc.entry_points;

        for (const auto& entry : desc.entry_points)
            data.modules.emplace(entry.stage, CompileShaderModule(desc.path, buffer, desc.lang, entry, desc.defines));

        data.group_layout = CreateResourceGroupLayout({ .bindings = desc.resource_bindings });

        ShaderHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::Shader, m_shaders.size() - 1, 0);
        return handle;
    }

    BufferHandle Driver::CreateBuffer(const BufferDescription& desc)
    {
        BufferData& data = m_buffers.emplace_back(nullptr, nullptr, desc);

        vk::BufferCreateInfo info{};
        info.size = desc.size;
        info.usage = ToVulkanBufferUsage(desc.usage);
        info.sharingMode = ToVulkanSharingMode(desc.share_mode);

        data.buffer = vk::raii::Buffer(m_logical_device, info);

        BufferHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::Buffer, m_buffers.size() - 1, 0);
        return handle;
    }

    TextureHandle Driver::CreateTexture(const TextureDescription& desc)
    {
        TextureData& data = m_textures.emplace_back(nullptr, nullptr, desc);

        vk::ImageCreateInfo info{};
        info.imageType = ToVulkanImageType(desc.type);
        info.format = ToVulkanFormat(desc.format);
        info.extent = vk::Extent3D(std::max<u32>(desc.extent.width, 1), std::max<u32>(desc.extent.height, 1), std::max<u32>(desc.extent.depth, 1));
        info.mipLevels = std::max<u32>(desc.mip_levels, 1);
        info.arrayLayers = std::max<u32>(desc.array_layers, 1);
        info.samples = ToVulkanSampleCount(desc.samples);
        info.tiling = ToVulkanImageTiling(desc.tiling);
        info.usage = ToVulkanTextureUsage(desc.usage);
        info.sharingMode = ToVulkanSharingMode(desc.sharing_mode);
        info.initialLayout = ToVulkanImageLayout(desc.initial_layout);

        data.image = vk::raii::Image(m_logical_device, info);

        TextureHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::Texture, m_textures.size() - 1, 0);
        return handle;
    }

    TextureViewHandle Driver::CreateTextureView(const TextureViewDescription& desc)
    {
        if (desc.texture.value == 0)
        {
            AURION_ERROR("[VulkanDriver::CreateTextureView] Failed to generate a view for the provided texture: Invalid texture handle.");
            return {};
        }

        const TextureData& texture_data = m_textures[GPUHandleIndex(desc.texture)];
        TextureViewData& data = m_texture_views.emplace_back(nullptr, desc);

        vk::ImageViewCreateInfo info{};
        info.image = *texture_data.image;
        info.viewType = ToVulkanImageViewType(desc.view_type);
        info.format = ToVulkanFormat(desc.format);
        info.components = vk::ComponentMapping(
            ToVulkanSwizzle(desc.components.r), ToVulkanSwizzle(desc.components.g),
            ToVulkanSwizzle(desc.components.b), ToVulkanSwizzle(desc.components.a));
        info.subresourceRange = vk::ImageSubresourceRange(
            ToVulkanImageAspect(desc.subresource_range.aspect_mask),
            desc.subresource_range.base_mip_level, std::max<u32>(desc.subresource_range.level_count, 1),
            desc.subresource_range.base_array_layer, std::max<u32>(desc.subresource_range.layer_count, 1));

        data.view = vk::raii::ImageView(m_logical_device, info);

        TextureViewHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::TextureView, m_texture_views.size() - 1, 0);
        return handle;
    }

    SamplerHandle Driver::CreateSampler(const SamplerDescription& desc)
    {
        SamplerData& data = m_samplers.emplace_back(nullptr, desc);

        // SamplerDescription is currently an empty placeholder on the interface - build a reasonable default.
        vk::SamplerCreateInfo info{};
        info.magFilter = ToVulkanFilter(desc.mag_filter);
        info.minFilter = ToVulkanFilter(desc.min_filter);
        info.addressModeU = ToVulkanAddressMode(desc.address_mode_u);
        info.addressModeV = ToVulkanAddressMode(desc.address_mode_v);
        info.addressModeW = ToVulkanAddressMode(desc.address_mode_w);
        info.borderColor = ToVulkanBorderColor(desc.border_color);
        info.mipLodBias = desc.mip_lod_bias;
        info.maxAnisotropy = desc.max_anisotropy;
        info.minLod = desc.min_lod;
        info.maxLod = desc.max_lod;
        info.anisotropyEnable = desc.enable_anisotropy;
        info.compareEnable = desc.enable_compare;
        info.compareOp = ToVulkanCompareOp(desc.compare_op);

        data.sampler = vk::raii::Sampler(m_logical_device, info);

        SamplerHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::Sampler, m_samplers.size() - 1, 0);
        return handle;
    }

    RenderTargetHandle Driver::CreateRenderTarget(const RenderTargetDescription& desc)
    {
        RenderTargetData& data = m_render_targets.emplace_back(
            SwapchainHandle(), TextureHandle(), TextureViewHandle(), desc
        );

        if (desc.surface.value != 0)
            data.swapchain = CreateSwapchain(desc.surface, desc.image_desc, desc.view_desc);
        else
        {
            data.image = CreateTexture(desc.image_desc);
            data.view = CreateTextureView({
                .texture = data.image,
                .view_type = desc.view_desc.view_type,
                .format = desc.view_desc.format,
                .subresource_range = desc.view_desc.subresource_range,
                .components = desc.view_desc.components,
            });
        }

        RenderTargetHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::RenderTarget, m_render_targets.size() - 1, 0);
        return handle;
    }

    ResourcePoolHandle Driver::CreateResourcePool(const ResourcePoolDescription& desc)
    {
        std::vector<vk::DescriptorPoolSize> pool_sizes{};
        pool_sizes.reserve(desc.group_sizes.size());
        for (const auto& gs : desc.group_sizes)
            pool_sizes.push_back({ ToVulkanDescriptorType(gs.type), gs.count * desc.max_groups });

        vk::DescriptorPoolCreateFlags flags{};
        if (desc.attributes & ResourcePoolAttributes::AllowGroupFree)
            flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

        vk::DescriptorPoolCreateInfo info{};
        info.flags = flags;
        info.maxSets = desc.max_groups;
        info.poolSizeCount = static_cast<u32>(pool_sizes.size());
        info.pPoolSizes = pool_sizes.data();

        ResourcePoolData& data = m_resource_pools.emplace_back(nullptr, desc);
        data.pool = vk::raii::DescriptorPool(m_logical_device, info);

        ResourcePoolHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::ResourcePool, m_resource_pools.size() - 1, 0);
        return handle;
    }

    ResourceGroupLayoutHandle Driver::CreateResourceGroupLayout(const ResourceGroupLayoutDescription& desc)
    {
        std::vector<vk::DescriptorSetLayoutBinding> vk_bindings{};
        vk_bindings.reserve(desc.bindings.size());

        for (const auto& binding : desc.bindings)
        {
            vk::DescriptorSetLayoutBinding vk_binding{};
            vk_binding.binding = binding.binding;
            vk_binding.descriptorType = ToVulkanDescriptorType(binding.type);
            vk_binding.descriptorCount = binding.count;
            vk_binding.stageFlags = ToVulkanShaderStageFlags(binding.stage_flags);
            vk_bindings.push_back(vk_binding);
        }

        vk::DescriptorSetLayoutCreateInfo info{};
        info.bindingCount = static_cast<u32>(vk_bindings.size());
        info.pBindings = vk_bindings.data();

        ResourceGroupLayoutData& data = m_resource_group_layouts.emplace_back(nullptr, desc);
        data.layout = vk::raii::DescriptorSetLayout(m_logical_device, info);

        ResourceGroupLayoutHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::ResourceGroupLayout, m_resource_group_layouts.size() - 1, 0);
        return handle;
    }

    ResourceGroupHandle Driver::AllocateResourceGroup(const ResourcePoolHandle& pool, const ResourceGroupLayoutHandle& layout)
    {
        const std::vector<ResourceGroupHandle> groups = AllocateResourceGroups(pool, layout, 1);
        return groups.empty() ? ResourceGroupHandle{} : groups.front();
    }

    // --- Batch Resource Allocation ---

    std::vector<PipelineHandle> Driver::CreatePipelines(std::span<PipelineDescription> descriptions)
    {
        std::vector<PipelineHandle> out{};
        out.reserve(descriptions.size());
        for (const auto& desc : descriptions) out.push_back(CreatePipeline(desc));
        return out;
    }

    std::vector<ShaderHandle> Driver::CreateShaders(std::span<ShaderDescription> descriptions)
    {
        std::vector<ShaderHandle> out{};
        out.reserve(descriptions.size());
        for (const auto& desc : descriptions) out.push_back(CreateShader(desc));
        return out;
    }

    std::vector<BufferHandle> Driver::CreateBuffers(std::span<BufferDescription> descriptions)
    {
        std::vector<BufferHandle> out{};
        out.reserve(descriptions.size());
        for (const auto& desc : descriptions) out.push_back(CreateBuffer(desc));
        return out;
    }

    std::vector<TextureHandle> Driver::CreateTextures(std::span<TextureDescription> descriptions)
    {
        std::vector<TextureHandle> out{};
        out.reserve(descriptions.size());
        for (const auto& desc : descriptions) out.push_back(CreateTexture(desc));
        return out;
    }

    std::vector<TextureViewHandle> Driver::CreateTextureViews(std::span<TextureViewDescription> descriptions)
    {
        std::vector<TextureViewHandle> out{};
        out.reserve(descriptions.size());
        for (const auto& desc : descriptions) out.push_back(CreateTextureView(desc));
        return out;
    }

    std::vector<SamplerHandle> Driver::CreateSamplers(std::span<SamplerDescription> descriptions)
    {
        std::vector<SamplerHandle> out{};
        out.reserve(descriptions.size());
        for (const auto& desc : descriptions) out.push_back(CreateSampler(desc));
        return out;
    }

    std::vector<RenderTargetHandle> Driver::CreateRenderTargets(std::span<RenderTargetDescription> descriptions)
    {
        std::vector<RenderTargetHandle> out{};
        out.reserve(descriptions.size());
        for (const auto& desc : descriptions) out.push_back(CreateRenderTarget(desc));
        return out;
    }

    std::vector<ResourcePoolHandle> Driver::CreateResourcePools(std::span<ResourcePoolDescription> descriptions)
    {
        std::vector<ResourcePoolHandle> out{};
        out.reserve(descriptions.size());
        for (const auto& desc : descriptions) out.push_back(CreateResourcePool(desc));
        return out;
    }

    std::vector<ResourceGroupLayoutHandle> Driver::CreateResourceGroupLayouts(std::span<ResourceGroupLayoutDescription> descriptions)
    {
        std::vector<ResourceGroupLayoutHandle> out{};
        out.reserve(descriptions.size());
        for (const auto& desc : descriptions) out.push_back(CreateResourceGroupLayout(desc));
        return out;
    }

    std::vector<ResourceGroupHandle> Driver::AllocateResourceGroups(const ResourcePoolHandle& pool, const ResourceGroupLayoutHandle& layout, const u32& count)
    {
        if (pool.value == 0 || layout.value == 0 || count == 0)
            return {};

        std::vector<ResourceGroupHandle> out{};
        ResourcePoolData& pool_data = m_resource_pools[GPUHandleIndex(pool)];
        ResourceGroupLayoutData& layout_data = m_resource_group_layouts[GPUHandleIndex(layout)];

        std::vector<vk::DescriptorSetLayout> layouts(count, *layout_data.layout);

        vk::DescriptorSetAllocateInfo alloc_info{};
        alloc_info.descriptorPool = *pool_data.pool;
        alloc_info.descriptorSetCount = count;
        alloc_info.pSetLayouts = layouts.data();

        std::vector<vk::raii::DescriptorSet> sets = m_logical_device.allocateDescriptorSets(alloc_info);

        out.reserve(sets.size());
        for (auto& set : sets)
        {
            ResourceGroupData& group_data = m_resource_groups.emplace_back(nullptr, layout);
            group_data.set = std::move(set);

            ResourceGroupHandle handle{};
            handle.value = MakeGPUHandleValue(GPUResourceType::ResourceGroup, m_resource_groups.size() - 1, 0);
            out.push_back(handle);
        }

        return out;
    }

    // --- Resource Cleanup ---

    void Driver::Release(GPUHandle& handle)
    {
        switch (GPUHandleType(handle))
        {
            case GPUResourceType::Buffer: m_buffers.erase(m_buffers.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::Texture: m_textures.erase(m_textures.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::TextureView: m_texture_views.erase(m_texture_views.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::Sampler: m_samplers.erase(m_samplers.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::Shader: m_shaders.erase(m_shaders.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::Pipeline: m_pipelines.erase(m_pipelines.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::RenderTarget: m_render_targets.erase(m_render_targets.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::Surface: m_surfaces.erase(m_surfaces.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::ResourcePool: m_resource_pools.erase(m_resource_pools.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::ResourceGroupLayout: m_resource_group_layouts.erase(m_resource_group_layouts.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::ResourceGroup: m_resource_groups.erase(m_resource_groups.begin() + GPUHandleIndex(handle)); break;
            case GPUResourceType::ResourceMemory: m_resource_memory.erase(m_resource_memory.begin() + GPUHandleIndex(handle)); break;
            default: break;
        }

        handle.value = 0;
    }

    // --- Resource & Memory Binding ---

    void Driver::UpdateResourceGroupBinding(const ResourceBindingUpdate& update)
    {
        ResourceBindingUpdate copy = update;
        UpdateResourceGroupBindings(std::span<ResourceBindingUpdate>(&copy, 1));
    }

    void Driver::UpdateResourceGroupBindings(std::span<ResourceBindingUpdate> updates)
    {
        // Resolve each update's real descriptor type from the target resource group's layout - the
        // update's own `type` field is commonly left as GPUResourceType::Unknown by callers, since
        // the layout (built from ResourceGroupLayoutDescription::bindings) already declares it.
        auto resolve_type = [&](const ResourceBindingUpdate& update) -> GPUResourceType
        {
            if (update.type != GPUResourceType::Unknown || update.group.value == 0)
                return update.type;

            const ResourceGroupData& group = m_resource_groups[GPUHandleIndex(update.group)];

            if (group.layout.value == 0) return update.type;

            const ResourceGroupLayoutData& layout = m_resource_group_layouts[GPUHandleIndex(group.layout)];

            const auto it = std::ranges::find_if(layout.desc.bindings, [&](const auto& b) { return b.binding == update.binding; });
            return it != layout.desc.bindings.end() ? it->type : update.type;
        };

        u64 total_buffer_infos = 0, total_image_infos = 0;
        for (const auto& update : updates)
        {
            const auto type = resolve_type(update);
            if (type >= GPUResourceType::Buffer && type <= GPUResourceType::UniformTexelBuffer)
                total_buffer_infos += update.resources.size();
            else total_image_infos += update.resources.size();
        }

        // Reserved up-front so pointers taken into these below stay valid for the final call.
        std::vector<vk::DescriptorBufferInfo> buffer_infos{};
        std::vector<vk::DescriptorImageInfo> image_infos{};
        buffer_infos.reserve(total_buffer_infos);
        image_infos.reserve(total_image_infos);

        std::vector<vk::WriteDescriptorSet> writes{};
        writes.reserve(updates.size());

        for (const auto& update : updates)
        {
            if (update.group.value == 0 || update.resources.empty()) continue;

            const ResourceGroupData& group = m_resource_groups[GPUHandleIndex(update.group)];
            const GPUResourceType resolved_type = resolve_type(update);

            vk::WriteDescriptorSet write{};
            write.dstSet = *group.set;
            write.dstBinding = update.binding;
            write.dstArrayElement = update.first_index;
            write.descriptorType = ToVulkanDescriptorType(resolved_type);
            write.descriptorCount = static_cast<u32>(update.resources.size());

            // If the type is a buffer type
            if (resolved_type >= GPUResourceType::Buffer && resolved_type <= GPUResourceType::UniformTexelBuffer)
            {
                const u64 start = buffer_infos.size();
                for (const auto& res : update.resources)
                {
                    vk::DescriptorBufferInfo info{};
                    if (res.value != 0)
                    {
                        const BufferData& buffer_data = m_buffers[GPUHandleIndex(res)];
                        info.buffer = *buffer_data.buffer;
                        info.offset = 0;
                        info.range = buffer_data.desc.size;
                    }
                    buffer_infos.push_back(info);
                }
                write.pBufferInfo = &buffer_infos[start];
            }
            else
            {
                const u64 start = image_infos.size();
                for (const auto& res : update.resources)
                {
                    vk::DescriptorImageInfo info{};
                    info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

                    if (GPUHandleType(res) == GPUResourceType::Sampler && res.value != 0)
                    {
                        const SamplerData& sampler_data = m_samplers[GPUHandleIndex(res)];
                        info.sampler = *sampler_data.sampler;
                    }
                    else if (res.value != 0)
                    {
                        const TextureViewData& view_data = m_texture_views[GPUHandleIndex(res)];
                        info.imageView = *view_data.view;
                    }

                    image_infos.push_back(info);
                }
                write.pImageInfo = &image_infos[start];
            }

            writes.push_back(write);
        }

        if (writes.empty()) return;

        m_logical_device.updateDescriptorSets(writes, {});
    }

    ResourceMemoryRequirements Driver::QueryMemoryRequirements(const BufferDescription& buffer_desc)
    {
        vk::BufferCreateInfo info{};
        info.size = std::max<u32>(buffer_desc.size, 1);
        info.usage = ToVulkanBufferUsage(buffer_desc.usage);
        info.sharingMode = ToVulkanSharingMode(buffer_desc.share_mode);

        vk::DeviceBufferMemoryRequirements buffer_requirements{};
        buffer_requirements.pCreateInfo = &info;

        auto query_chain = m_logical_device.getBufferMemoryRequirements<vk::MemoryRequirements2, vk::MemoryDedicatedRequirements>(buffer_requirements);

        const vk::MemoryRequirements& reqs = query_chain.get<vk::MemoryRequirements2>().memoryRequirements;
        const vk::MemoryDedicatedRequirements dedi_reqs = query_chain.get<vk::MemoryDedicatedRequirements>();

        return ResourceMemoryRequirements{
            .size = reqs.size,
            .alignment = reqs.alignment,
            .memory_type_mask = reqs.memoryTypeBits,
            .prefers_dedicated_memory = dedi_reqs.prefersDedicatedAllocation == VK_TRUE,
            .requires_dedicated_memory = dedi_reqs.requiresDedicatedAllocation == VK_TRUE,
        };
    }

    ResourceMemoryRequirements Driver::QueryMemoryRequirements(const RenderTargetDescription& render_target_desc)
    {
        const auto& img = render_target_desc.image_desc;

        vk::ImageCreateInfo info{};
        info.imageType = ToVulkanImageType(img.type);
        info.format = ToVulkanFormat(img.format);
        info.extent = vk::Extent3D(std::max<u32>(img.extent.width, 1), std::max<u32>(img.extent.height, 1), std::max<u32>(img.extent.depth, 1));
        info.mipLevels = std::max<u32>(img.mip_levels, 1);
        info.arrayLayers = std::max<u32>(img.array_layers, 1);
        info.samples = ToVulkanSampleCount(img.samples);
        info.tiling = ToVulkanImageTiling(img.tiling);
        info.usage = ToVulkanTextureUsage(img.usage);
        info.sharingMode = ToVulkanSharingMode(img.sharing_mode);
        info.initialLayout = ToVulkanImageLayout(img.initial_layout);

        vk::DeviceImageMemoryRequirements img_requirements{};
        img_requirements.pCreateInfo = &info;

        auto query_chain = m_logical_device.getImageMemoryRequirements<vk::MemoryRequirements2, vk::MemoryDedicatedRequirements>(img_requirements);

        const vk::MemoryRequirements& reqs = query_chain.get<vk::MemoryRequirements2>().memoryRequirements;
        const vk::MemoryDedicatedRequirements dedi_reqs = query_chain.get<vk::MemoryDedicatedRequirements>();

        return ResourceMemoryRequirements{
            .size = reqs.size,
            .alignment = reqs.alignment,
            .memory_type_mask = reqs.memoryTypeBits,
            .prefers_dedicated_memory = dedi_reqs.prefersDedicatedAllocation == VK_TRUE,
            .requires_dedicated_memory = dedi_reqs.requiresDedicatedAllocation == VK_TRUE,
        };
    }

    ResourceMemoryHandle Driver::AllocateResourceMemory(const ResourceMemoryRequirements& requirements)
    {
        vk::MemoryRequirements reqs{};
        reqs.size = requirements.size;
        reqs.alignment = requirements.alignment;
        reqs.memoryTypeBits = requirements.memory_type_mask;

        ResourceMemoryData& data = m_resource_memory.emplace_back(nullptr, requirements.size);
        data.memory = AllocateDeviceMemory(reqs, vk::MemoryPropertyFlagBits::eDeviceLocal);

        ResourceMemoryHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::ResourceMemory, m_resource_memory.size() - 1, 0);
        return handle;
    }

    void Driver::BindResourceMemory(const GPUHandle& resource, const ResourceMemoryHandle& memory, const u64& offset)
    {
        if (resource.value == 0) return;

        ResourceMemoryData& memory_data = m_resource_memory[GPUHandleIndex(memory)];

        switch (GPUHandleType(resource))
        {
            case GPUResourceType::Buffer:
            {
                BufferData& data = m_buffers[GPUHandleIndex(resource)];
                data.memory = memory_data.memory;
                data.buffer.bindMemory(**memory_data.memory, offset);
                break;
            }
            case GPUResourceType::Texture:
            {
                TextureData& data = m_textures[GPUHandleIndex(resource)];
                data.memory = memory_data.memory;
                data.image.bindMemory(**memory_data.memory, offset);
                break;
            }
            case GPUResourceType::RenderTarget:
            {
                RenderTargetData& data = m_render_targets[GPUHandleIndex(resource)];

                // Don't attempt to bind to swapchain images, or invalid images
                if (data.swapchain.value != 0 || data.image.value == 0) break;

                // Then, bind memory to internal image
                BindResourceMemory(data.image, memory, offset);
                break;
            }
            default: break;
        }
    }

    // --- Resource Handle Retrieval ---

    ResourceGroupLayoutHandle Driver::GetShaderResourceGroupLayout(const ShaderHandle& shader)
    {
        if (shader.value == 0) return {};
        return m_shaders[GPUHandleIndex(shader)].group_layout;
    }

    SwapchainHandle Driver::CreateSwapchain(const SurfaceHandle& surface, const TextureDescription& image_desc,
        const TextureViewDescription& view_desc)
    {
        if (surface.value == 0) // Can't create a swapchain from an invalid surface
        {
            AURION_ERROR("[VulkanDriver::CreateSwapchain] Failed to create swapchain: Invalid surface handle!");
            return {};
        }

        SwapchainData& data = m_swapchains.emplace_back(surface);

        // TODO: Might be worthwhile to check surface generation here

        // Retrieve SurfaceData and WindowProperties
        const SurfaceData& surface_data = m_surfaces[GPUHandleIndex(surface)];
        const WindowProperties& win_props = surface_data.desc.window->GetProperties();

        // Query Surface capabilities
        const auto capabilities = m_physical_device.getSurfaceCapabilitiesKHR(*surface_data.surface);
        const std::vector<vk::SurfaceFormatKHR> available_formats = m_physical_device.getSurfaceFormatsKHR(*surface_data.surface);
        const std::vector<vk::PresentModeKHR> available_present_modes = m_physical_device.getSurfacePresentModesKHR(*surface_data.surface);

        // Ensure the surface has an available format
        if (available_formats.empty())
            throw std::runtime_error("[VulkanDriver::CreateSwapchain] No available surface formats.");

        // Choose an available format and color space
        const vk::Format desired_format = ToVulkanFormat(view_desc.format);
        const vk::ColorSpaceKHR desired_color_space = ToVulkanColorSpace(surface_data.desc.color_space);

        const auto format_it = std::ranges::find_if(available_formats, [&](const auto& f)
        {
            return f.format == desired_format && f.colorSpace == desired_color_space;
        });
        const vk::SurfaceFormatKHR chosen_format = format_it != available_formats.end() ? *format_it : available_formats.front();

        const vk::PresentModeKHR desired_present_mode = ToVulkanPresentMode(surface_data.desc.present_mode);
        const vk::PresentModeKHR chosen_present_mode = std::ranges::any_of(available_present_modes,
            [&](const auto& mode) { return mode == desired_present_mode; }) ? desired_present_mode : vk::PresentModeKHR::eFifo;

        // Figure out the image extent, based on surface capabilities and window size
        vk::Extent2D chosen_extent{};
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
            chosen_extent = capabilities.currentExtent;
        else
            chosen_extent = vk::Extent2D{
            std::clamp<u32>(win_props.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<u32>(win_props.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };

        // Then, figure out how many swapchain images are required
        u32 desired_img_count = 0;
        switch (surface_data.desc.present_mode)
        {
            case PresentMode::Immediate: { desired_img_count = 1; break; }
            case PresentMode::DoubleBufferedVSync: { desired_img_count = 2; break; }
            case PresentMode::TripleBufferedVSync: { desired_img_count = 3; break; }
            case PresentMode::QuadBufferedVSync: { desired_img_count = 4; break; }
            case PresentMode::DoubleBufferedLowLatency: { desired_img_count = 2; break; }
            case PresentMode::TripleBufferedLowLatency: { desired_img_count = 3; break; }
            case PresentMode::QuadBufferedLowLatency: { desired_img_count = 4; break; }
            default: { desired_img_count = 1; } // Default to immediate-mode rendering
        }

        u32 min_image_count = std::max<u32>(desired_img_count, capabilities.minImageCount + 1);
        if (capabilities.maxImageCount > 0 && min_image_count > capabilities.maxImageCount)
            min_image_count = capabilities.maxImageCount;

        // Then, create the swapchain
        vk::SwapchainCreateInfoKHR sc_info{};
        sc_info.surface = *surface_data.surface;
        sc_info.minImageCount = min_image_count;
        sc_info.imageFormat = chosen_format.format;
        sc_info.imageColorSpace = chosen_format.colorSpace;
        sc_info.imageExtent = chosen_extent;
        sc_info.imageArrayLayers = 1;
        sc_info.imageUsage = ToVulkanTextureUsage(image_desc.usage);
        sc_info.imageSharingMode = ToVulkanSharingMode(image_desc.sharing_mode);
        sc_info.preTransform = capabilities.currentTransform;
        sc_info.compositeAlpha = ToVulkanCompositeAlpha(surface_data.desc.composite_alpha);
        sc_info.presentMode = chosen_present_mode;
        sc_info.clipped = surface_data.desc.clipped;

        data.swapchain = vk::raii::SwapchainKHR(m_logical_device, sc_info, nullptr);

        // Retrieve swapchain images and reserve space for views/semaphores
        data.images = data.swapchain.getImages();
        data.views.reserve(data.images.size());
        data.acquire_semaphores.reserve(data.images.size());
        data.present_semaphores.reserve(data.images.size());

        // Finally, allocate views/semaphores
        for (const auto& image : data.images)
        {
            vk::ImageViewCreateInfo view_info{};
            view_info.image = image;
            view_info.viewType = vk::ImageViewType::e2D;
            view_info.format = chosen_format.format;
            view_info.components = vk::ComponentMapping(
                ToVulkanSwizzle(view_desc.components.r), ToVulkanSwizzle(view_desc.components.g),
                ToVulkanSwizzle(view_desc.components.b), ToVulkanSwizzle(view_desc.components.a));
            view_info.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

            data.views.emplace_back(m_logical_device, view_info);

            constexpr vk::SemaphoreCreateInfo sem_info{};
            data.acquire_semaphores.emplace_back(m_logical_device, sem_info);
            data.present_semaphores.emplace_back(m_logical_device, sem_info);
        }

        // Generate the handle
        SwapchainHandle handle{};
        handle.value = MakeGPUHandleValue(GPUResourceType::Swapchain, m_swapchains.size() - 1, 0);

        return handle;
    }

    // --- Buffer Operations ---

    // --- Private helpers ---

    vk::raii::ShaderModule Driver::CompileShaderModule(
        const std::string& path,
        const std::vector<char>& code,
        const ShaderLanguage& lang,
        const ShaderEntryPoint& entry_point,
        const std::vector<ShaderMacro>& defines
    ) const
    {
        // SPIR-V can be used directly
        if (lang == ShaderLanguage::SPIRV)
        {
            vk::ShaderModuleCreateInfo info{};
            info.codeSize = code.size();
            info.pCode = reinterpret_cast<const u32*>(code.data());
            return vk::raii::ShaderModule(m_logical_device, info);
        }

        // GLSL/HLSL must be compiled
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        if (lang == ShaderLanguage::GLSL)
            options.SetSourceLanguage(shaderc_source_language_glsl);
        else if (lang == ShaderLanguage::HLSL)
            options.SetSourceLanguage(shaderc_source_language_hlsl);

        for (const auto& macro : defines)
            options.AddMacroDefinition(macro.key, macro.value);

        shaderc_shader_kind kind = shaderc_vertex_shader;
        switch (entry_point.stage)
        {
            case ShaderStage::Vertex: kind = shaderc_vertex_shader; break;
            case ShaderStage::TessellationControl: kind = shaderc_tess_control_shader; break;
            case ShaderStage::TessellationEval: kind = shaderc_tess_evaluation_shader; break;
            case ShaderStage::Geometry: kind = shaderc_geometry_shader; break;
            case ShaderStage::Fragment: kind = shaderc_fragment_shader; break;
            case ShaderStage::Task: kind = shaderc_task_shader; break;
            case ShaderStage::Mesh: kind = shaderc_mesh_shader; break;
            default: break;
        }

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
            code.data(), code.size(), kind, path.c_str(), entry_point.id.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
            throw std::runtime_error("[VulkanDriver] Failed to compile '" + path + "' into SPIR-V: " + result.GetErrorMessage());

        const std::vector<u32> spv(result.cbegin(), result.cend());

        vk::ShaderModuleCreateInfo info{};
        info.codeSize = spv.size() * sizeof(u32);
        info.pCode = spv.data();

        return vk::raii::ShaderModule(m_logical_device, info);
    }

    std::shared_ptr<vk::raii::DeviceMemory> Driver::AllocateDeviceMemory(
        const vk::MemoryRequirements& mem_reqs,
        const vk::MemoryPropertyFlags& prop_flags
    ) const
    {
        const vk::PhysicalDeviceMemoryProperties props = m_physical_device.getMemoryProperties();

        u32 mem_type_index = UINT32_MAX;
        for (u32 i = 0; i < props.memoryTypeCount; ++i)
            if ((mem_reqs.memoryTypeBits & (1u << i)) && (props.memoryTypes[i].propertyFlags & prop_flags) == prop_flags)
                mem_type_index = i;

        if (mem_type_index == UINT32_MAX)
            throw std::runtime_error("[VulkanDriver] Failed to allocate device memory: no suitable memory type.");

        vk::MemoryAllocateInfo info{};
        info.allocationSize = mem_reqs.size;
        info.memoryTypeIndex = mem_type_index;

        return std::make_shared<vk::raii::DeviceMemory>(m_logical_device, info);
    }

    vk::raii::SurfaceKHR Driver::CreateWindowSurface(Window* window) const
    {
        if (!window)
            throw std::runtime_error("[VulkanDriver] CreateWindowSurface: null window.");

        VkSurfaceKHR raw_surface = VK_NULL_HANDLE;
        const VkResult result = glfwCreateWindowSurface(
            *m_vulkan_api->GetInstance(), static_cast<GLFWwindow*>(window->GetNativeHandle()), nullptr, &raw_surface);

        if (result != VK_SUCCESS)
            throw std::runtime_error("[VulkanDriver] Failed to create window surface.");

        return vk::raii::SurfaceKHR(m_vulkan_api->GetInstance(), raw_surface);
    }

    void Driver::ValidatePresentSupport(const vk::raii::SurfaceKHR& surface) const
    {
        for (const auto& family : m_queue_families | std::views::values)
            if (m_physical_device.getSurfaceSupportKHR(family.index, *surface))
                return;

        throw std::runtime_error("[VulkanDriver] Failed to validate presentation support: provided surface does not support image presentation.");
    }
}
