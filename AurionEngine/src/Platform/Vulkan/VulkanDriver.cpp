module;

#include <AurionLog.h>
#include <vulkan/vulkan_raii.hpp>
#include <shaderc/shaderc.hpp>

#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <ranges>
#include <string>
#include <limits>
#include <algorithm>

module Aurion.Vulkan;

import Aurion.Services;
import Aurion.Types;

namespace Aurion::Vulkan
{
    Driver::Driver(const DriverConfig& config)
        : m_physical_device(config.physical_device),
            m_logical_device(nullptr), m_CreateSurfaceFn(config.on_surface_create)
    {
        // Ensure access to application resources
        m_resource_manager = ServiceLocator::GetService<ResourceManager>();

        // Attempt to retrieve the Vulkan API Service
        Vulkan::API* api = ServiceLocator::GetService<Vulkan::API>();
        if (!api)
            throw std::runtime_error("[Vulkan Driver] Vulkan API Service is unavailable");

        // If available, grab references to the context and instance
        m_context = &api->GetContext();
        m_instance = &api->GetInstance();

        // Create the logical device for interfacing with the physical device
        // ------------------------------------------------------------------

        // Get all physical device queue descriptions
        std::vector<vk::QueueFamilyProperties> device_qfp = m_physical_device.getQueueFamilyProperties();

        // Track which queue descriptions belong to which queue family
        std::unordered_map<u32, std::vector<QueueDescription>> queue_family_desc;

        // Aggregate queue descriptions into their most optimal queue family
        for (const auto& desc : config.interface.queues)
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
                throw std::runtime_error("[Vulkan Driver] No queue family with bit flag [value of \" + desc.flags + \"] was found!");

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
            if (!queue_family_desc.contains(best_index))
                queue_family_desc[best_index] = { desc };
            else
                queue_family_desc[best_index].push_back(desc);
        }

        // Once queue descriptions have been aggregated by queue family, flatten all descriptions into
        //  one queue description per queue family
        std::unordered_map<u32, QueueDescription> qf_infos;
        for (const auto& [index, desc_arr] : queue_family_desc)
        {
            // Assign a new aggregate queue description
            qf_infos[index] = {
                .count = 0,
                .priorities = {}
            };

            // For each unique queue family
            for (const auto qf_desc : desc_arr)
            {
                // Increase the number of queues of this family to create
                qf_infos[index].count += qf_desc.count;

                // Append all queue priorities
                qf_infos[index].priorities.append_range(qf_desc.priorities);
            }
        }

        // After flattening, generate DeviceQueueCreateInfo structures
        std::vector<vk::DeviceQueueCreateInfo> create_queues{};
        for (const auto& [index, desc] : qf_infos)
        {
            AURION_WARN("Queue Family Index [%d]: Creating %d queues.", index, desc.count);

            vk::DeviceQueueCreateInfo cInfo{};
            cInfo.queueFamilyIndex = static_cast<u32>(index);
            cInfo.queueCount = static_cast<u32>(desc.count);
            cInfo.pQueuePriorities = desc.priorities.data();

            create_queues.push_back(cInfo);
        }

        // Then, generate the logical device
        vk::DeviceCreateInfo dcInfo{};
        dcInfo.pNext = config.interface.features;
        dcInfo.queueCreateInfoCount = static_cast<u32>(create_queues.size());
        dcInfo.pQueueCreateInfos = create_queues.data();
        dcInfo.enabledExtensionCount = static_cast<u32>(config.interface.extensions.size());
        dcInfo.ppEnabledExtensionNames = config.interface.extensions.data();

        m_logical_device = vk::raii::Device(m_physical_device, dcInfo);

        // Allocate a command pool and command buffers for each queue in each queue family
        for (const auto& [index, desc] : qf_infos)
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

            // Allocate a command buffer for each queue in the family, scaled by the number of max in-flight frames
            qf.AllocateCommandBuffers(m_logical_device, vk::CommandBufferLevel::ePrimary);
        }
    }

    Driver::~Driver()
    {

    }

    void Driver::BeginFrame()
    {
        // Waits on the previous frame

        // Resets command buffer(s)
    }

    void Driver::RecordCommands()
    {
        // Iterates over all render passes

        // Records GPU operations on the command buffer(s) in question
    }

    void Driver::EndFrame()
    {

    }

    void Driver::ResolveFrameGraph(const FrameGraph& graph)
    {
        std::vector<FrameContext> contexts;

        std::vector<GraphicsResource::Config*> inputs{};
        std::vector<GraphicsResource::Config*> outputs{};
        for (const auto pass : graph.pass_descriptions)
        {
            // Get all input resource descriptions
            for (const auto& name : pass->inputs)
                for (const auto& desc : graph.resource_descriptions)
                    if (name == desc->name)
                        inputs.push_back(desc.get());

            // Get all output resource descriptions
            for (const auto& name : pass->outputs)
                for (const auto& desc : graph.resource_descriptions)
                    if (name == desc->name)
                        outputs.push_back(desc.get());

            // Create render pass context; Resolving the command buffer reference and resource handles
            // contexts.emplace_back(
            //     ResolveRenderPassCommandBuffer(pass->op_type, pass->channel_index, pass->command_buffer_index),
            //     ResolveRenderPassResources(inputs),
            //     ResolveRenderPassResources(outputs)
            // );

            inputs.clear();
            outputs.clear();
        }
    }

    ResourceHandle<Aurion::Buffer> Driver::CreateBuffer(const std::string_view& id)
    {
        // Buffers are stored as application resources
        auto handle = m_resource_manager->Load<Aurion::Buffer, Vulkan::Buffer>(id);

        // If the handle is invalid, simply return it. This should only happen when
        //  resource creation failed.
        if (!handle.IsValid()) return handle;

        // Attach this driver
        handle->Attach(this);

        return handle;
    }

    ResourceHandle<Aurion::RenderTarget> Driver::CreateRenderTarget(const std::string_view& id)
    {
        // Render targets get stored as application resources
        auto handle = m_resource_manager->Load<Aurion::RenderTarget, Vulkan::RenderTarget>(id);

        // If the handle is invalid, simply return it. This should only happen when
        //  resource creation failed.
        if (!handle.IsValid()) return handle;

        // Assign this driver
        handle->Attach(this);

        return handle;
    }

    ResourceHandle<Aurion::Shader> Driver::CreateShader(const std::string_view& id)
    {
        // Shaders get stored as application resources
        auto handle = m_resource_manager->Load<Aurion::Shader, Vulkan::Shader>(id);

        // If the handle is invalid, simply return it. This should only happen when
        //  resource creation failed.
        if (!handle.IsValid()) return handle;

        // Assign this driver
        handle->Attach(this);

        return handle;
    }

    ResourceHandle<Aurion::Pipeline> Driver::CreatePipeline(const std::string_view& id, const Pipeline::Type& type)
    {
        // Shaders get stored as application resources
        ResourceHandle<Aurion::Pipeline> handle;

        switch (type)
        {
        case Pipeline::Graphics:
            {
                handle = m_resource_manager->Load<Aurion::Pipeline, Vulkan::GraphicsPipeline>(id);
                break;
            }
        default:
            {
                // By default, return an invalid handle. This should only happen when
                //  resource creation failed, or hasn't been implemented.
                return handle;
            }
        }

        // Otherwise, Assign this driver
        handle->Attach(this);

        return handle;
    }

    vk::raii::SurfaceKHR Driver::CreateWindowSurface(Window* window) const
    {
        if (!m_CreateSurfaceFn)
        {
            AURION_WARN("[Vulkan Driver] Failed to create window surface: Surface creation has not been configured!");
            return vk::raii::SurfaceKHR(nullptr);
        }

        return vk::raii::SurfaceKHR(*m_instance, m_CreateSurfaceFn(*m_instance, window));
    }

    void Driver::ValidatePresentSupport(const vk::raii::SurfaceKHR& surface) const
    {
        for (const auto& family : m_queue_families | std::views::values)
        {
            if (!m_physical_device.getSurfaceSupportKHR(family.index, *surface))
                continue;

            // If found, return (for now; Might want to grab a reference to the queue in the future)
            return;
        }

        // If no queue was found, presentation isn't supported.
        throw std::runtime_error("[Vulkan Driver] Failed to validate presentation support: Provided surface does not support image presentation.");
    }

    vk::raii::SwapchainKHR Driver::CreateSwapchain(
            const vk::raii::SurfaceKHR& surface,
            const RenderTarget::Config& properties,
            vk::raii::SwapchainKHR* old_swapchain
    ) const {
        auto capabilities = m_physical_device.getSurfaceCapabilitiesKHR(*surface);

        std::vector<vk::SurfaceFormatKHR> available_formats = m_physical_device.getSurfaceFormatsKHR(*surface);
        std::vector<vk::PresentModeKHR> available_present_modes = m_physical_device.getSurfacePresentModesKHR(*surface);

        assert(!available_formats.empty() && "[Vulkan Driver] No available surface formats");

        // Attempt to find an SRGB format
        const auto formatIt = std::ranges::find_if(
            available_formats,
            [&](const auto& format)
            {
                return format.format == properties.image.format &&
                    format.colorSpace == properties.color_space;
            }
        );

        // Default to first available in all other cases
        const auto& chosen_format = formatIt != available_formats.end() ? *formatIt : available_formats.at(0);

        // Choose a presentation mode (Try to get fastest, most stable mode, defaulting to
        //      traditional vertical sync when not available)
        const auto& chosen_present_mode = std::ranges::any_of(
            available_present_modes,
            [&](const vk::PresentModeKHR value)
            {
                return properties.present_mode == value;
            }) ? properties.present_mode : vk::PresentModeKHR::eFifo;

        // Figuring out the swapchain extent
        vk::Extent2D chosen_extent{};

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            chosen_extent = capabilities.currentExtent;
        else
        {
            chosen_extent = vk::Extent2D{
                std::clamp<uint32_t>(properties.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                std::clamp<uint32_t>(properties.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
            };
        }

        // Figure out how many images to create: always at least 1 more than the minimum, or at most the maximum
        auto min_img_count = std::max(static_cast<u32>(properties.frames_in_flight), capabilities.minImageCount + 1);
        if ((capabilities.maxImageCount > 0) && (capabilities.maxImageCount < capabilities.minImageCount))
            min_img_count = capabilities.maxImageCount;

        vk::SwapchainCreateInfoKHR scInfo;
        scInfo.surface = *surface;
        scInfo.minImageCount = min_img_count;
        scInfo.imageFormat = chosen_format.format;
        scInfo.imageColorSpace = chosen_format.colorSpace;
        scInfo.imageExtent = chosen_extent;
        scInfo.imageArrayLayers = 1;
        scInfo.imageUsage = properties.image.usage;
        scInfo.imageSharingMode = properties.image.sharingMode;
        scInfo.preTransform = capabilities.currentTransform;
        scInfo.compositeAlpha = properties.composite_alpha;
        scInfo.presentMode = properties.vSync_enabled ? vk::PresentModeKHR::eFifo : chosen_present_mode;
        scInfo.clipped = properties.clipped;
        scInfo.oldSwapchain = *old_swapchain; // Used in swapchain recreation

        // Create swapchain and retrieve images
        return vk::raii::SwapchainKHR(m_logical_device, scInfo, nullptr);
    }

    std::vector<vk::raii::ImageView> Driver::CreateImageViews(
        const std::span<vk::Image>& images,
        const RenderTarget::Config& properties
    ) const {
        std::vector<vk::raii::ImageView> views{};

        // Copy image view configuration
        vk::ImageViewCreateInfo vcInfo = properties.view;

        // And apply for each image
        std::vector<vk::ImageViewCreateInfo> vcInfos{};
        for (auto& image : images)
        {
            vcInfo.image = image;
            views.emplace_back(m_logical_device, vcInfo);
        }

        return views;
    }

    vk::raii::Buffer Driver::AllocateBuffer(const Vulkan::Buffer::Config& config) const
    {
        vk::BufferCreateInfo bcInfo{};
        bcInfo.size = config.size;
        bcInfo.usage = config.usage;
        bcInfo.sharingMode = config.sharing_mode;

        return vk::raii::Buffer(m_logical_device, bcInfo);
    }

    vk::raii::DeviceMemory Driver::AllocateBufferMemory(
        const vk::raii::Buffer& buffer,
        vk::MemoryPropertyFlags prop_flags
    ) const
    {
        vk::MemoryRequirements reqs = buffer.getMemoryRequirements();
        vk::PhysicalDeviceMemoryProperties props = m_physical_device.getMemoryProperties();

        u32 mem_type_index = UINT32_MAX;
        for (u32 i = 0; i < props.memoryTypeCount; i++)
            if ((reqs.memoryTypeBits & (1 << i)) && (props.memoryTypes[i].propertyFlags & prop_flags) == prop_flags)
                mem_type_index = i;

        if (mem_type_index == UINT32_MAX)
            throw std::runtime_error("[Vulkan Driver] Failed to map buffer memory: No suitable memory type!");

        vk::MemoryAllocateInfo alloc_info{};
        alloc_info.allocationSize = reqs.size;
        alloc_info.memoryTypeIndex = mem_type_index;

        return vk::raii::DeviceMemory(m_logical_device, alloc_info);
    }

    vk::raii::ShaderModule Driver::CreateShaderModule(
        const std::string_view& path,
        const std::vector<char>& code,
        const Shader::Language& lang,
        const Shader::EntryPoint& entry_point,
        const std::vector<Shader::Macro>& defines
    ) const
    {
        // SPIR-V can be used directly
        if (lang == Shader::Language::SPIRV)
        {
            vk::ShaderModuleCreateInfo smcInfo{};
            smcInfo.codeSize = code.size() * sizeof(char);
            smcInfo.pCode = reinterpret_cast<const u32*>(code.data());

            vk::raii::ShaderModule module(m_logical_device, smcInfo);
            return module;
        }

        // GLSL/HLSL must be compiled
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        // Language Spec
        if (lang == Shader::Language::GLSL)
            options.SetSourceLanguage(shaderc_source_language_glsl);
        else if (lang == Shader::Language::HLSL)
            options.SetSourceLanguage(shaderc_source_language_hlsl);

        // Preprocessor Definitions
        for (const auto& [key, val] : defines)
            options.AddMacroDefinition(key, val);

        shaderc_shader_kind shader_kind = shaderc_vertex_shader;
        switch (entry_point.stage)
        {
            case Shader::Stage::Vertex: { shader_kind = shaderc_vertex_shader; break; }
            case Shader::TessellationControl: { shader_kind = shaderc_tess_control_shader; break; }
            case Shader::TessellationEval: { shader_kind = shaderc_tess_evaluation_shader; break; }
            case Shader::Geometry: { shader_kind = shaderc_geometry_shader; break; }
            case Shader::Fragment: { shader_kind = shaderc_fragment_shader; break; }
            case Shader::Task: { shader_kind = shaderc_task_shader; break; }
            case Shader::Mesh: { shader_kind = shaderc_mesh_shader; break; }
        }

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
            code.data(),
            code.size(),
            shader_kind,
            path.data(),
            entry_point.name.c_str(),
            options
        );

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
            throw std::runtime_error("[Vulkan Driver] Failed to compile '" + std::string(path) + "' into SPIR-V: " + result.GetErrorMessage());

        // Convert to uint32_t array
        std::vector<u32> spv = {result.cbegin(), result.cend()};
        // TODO: Save this result to Disk in a shader cache directory

        // Create the shader module
        vk::ShaderModuleCreateInfo smcInfo{};
        smcInfo.codeSize = spv.size() * sizeof(u32);
        smcInfo.pCode = spv.data();

        vk::raii::ShaderModule module(m_logical_device, smcInfo);
        return module;
    }

    vk::raii::PipelineLayout Driver::BuildPipelineLayout(const vk::PipelineLayoutCreateInfo& info) const
    {
        return vk::raii::PipelineLayout(m_logical_device, info);
    }

    vk::raii::Pipeline Driver::BuildGraphicsPipeline(const Vulkan::GraphicsPipeline::Config& config) const
    {
        std::vector<ResourceHandle<Aurion::Shader>> shader_handles;

        // Retrieve handles to all shaders
        for (const auto& name : config.shaders)
        {
            auto handle = m_resource_manager->Load<Aurion::Shader, Vulkan::Shader>(name);
            shader_handles.push_back(handle);
        }

        // Build create info structures
        std::vector<vk::PipelineShaderStageCreateInfo> stage_create_infos{};
        std::vector<const vk::raii::ShaderModule*> modules;
        for (auto& handle : shader_handles)
        {
            // Cast to Vulkan shader
            Vulkan::Shader* shader = dynamic_cast<Vulkan::Shader*>(handle.Get());

            for (const auto& entry : shader->GetEntryPoints())
            {
                // Get module reference
                modules.push_back(shader->GetModule(entry));

                // Build create info structure
                vk::PipelineShaderStageCreateInfo cInfo{};
                cInfo.module = *modules.back();
                cInfo.pName = entry.name.c_str();
                cInfo.stage = GetVulkanPipelineStage(entry.stage);

                stage_create_infos.push_back(cInfo);
            }
        }

        // Slice the config to fit the vulkan config structure
        vk::GraphicsPipelineCreateInfo cInfo(config);

        // Attach shader stage infos
        cInfo.stageCount = static_cast<u32>(stage_create_infos.size());
        cInfo.pStages = stage_create_infos.data();

        // TODO: Implement use of Pipeline Cache

        // Create the pipeline
        return vk::raii::Pipeline(m_logical_device, nullptr, cInfo, config.alloc_callbacks);
    }

    RenderPass::Resources Driver::ResolveRenderPassResources(const std::vector<GraphicsResource::Config*>& configs)
    {
        RenderPass::Resources resources{};

        for (const auto& config : configs)
        {
            switch (config->GetType())
            {
            case GraphicsResource::None:
                throw std::runtime_error("[Renderer] Failed to build render pipeline: Unknown resource type for \"" + config->name + "\".");
            case GraphicsResource::Buffer:
                {

                    break;
                }
            case GraphicsResource::RenderTarget:
                {
                    const auto handle = m_resource_manager->Load<RenderTarget>(config->name);
                    handle->Configure(static_cast<RenderTarget::Config*>(config)); // Configure render target
                    handle->Attach(nullptr); // Create internal render target resources
                    resources.render_targets.push_back(std::move(handle));
                    break;
                }
            case GraphicsResource::Texture:
                {

                    break;
                }
            case GraphicsResource::Sampler:
                {

                    break;
                }
            case GraphicsResource::Shader:
                break;
            case GraphicsResource::Pipeline:
                break;
            }
        }

        return resources;
    }

    vk::ShaderStageFlagBits Driver::GetVulkanPipelineStage(const Aurion::Shader::Stage& stage) const
    {
        switch (stage)
        {
            case Shader::Stage::Vertex: return vk::ShaderStageFlagBits::eVertex;
            case Shader::TessellationControl: return vk::ShaderStageFlagBits::eTessellationControl;
            case Shader::TessellationEval: return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case Shader::Geometry: return vk::ShaderStageFlagBits::eGeometry;
            case Shader::Fragment: return vk::ShaderStageFlagBits::eFragment;
            case Shader::Task: return vk::ShaderStageFlagBits::eTaskEXT;
            case Shader::Mesh: return vk::ShaderStageFlagBits::eMeshEXT;
            default: return vk::ShaderStageFlagBits::eVertex;
        }
    }
}
