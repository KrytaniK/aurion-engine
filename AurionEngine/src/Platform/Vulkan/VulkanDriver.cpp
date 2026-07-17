module;

#include <AurionLog.h>
#include <vulkan/vulkan_raii.hpp>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <ranges>

module Aurion.Vulkan;

import Aurion.Services;
import Aurion.Types;

namespace Aurion::Vulkan
{
    Driver::Driver(
        const vk::raii::PhysicalDevice& physical_device,
        const LogicalDeviceConfig& device_config)
        : m_physical_device(physical_device), m_logical_device(nullptr), m_present_queue(nullptr)
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
        std::unordered_map<u32, std::vector<LogicalDeviceQueueDescription>> queue_family_desc;

        // Aggregate queue descriptions into their most optimal queue family
        for (const auto& desc : device_config.queues)
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
        std::unordered_map<u32, LogicalDeviceQueueDescription> qf_infos;
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
        dcInfo.pNext = device_config.features;
        dcInfo.queueCreateInfoCount = static_cast<u32>(create_queues.size());
        dcInfo.pQueueCreateInfos = create_queues.data();
        dcInfo.enabledExtensionCount = static_cast<u32>(device_config.extensions.size());
        dcInfo.ppEnabledExtensionNames = device_config.extensions.data();

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

    void Driver::CreateRenderTarget(const Window* window)
    {

    }
}
