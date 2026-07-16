module;

#include <AurionLog.h>
#include <vulkan/vulkan_raii.hpp>
#include <stdexcept>
#include <vector>
#include <span>

module Aurion.Vulkan;

import Aurion.Services;
import Aurion.Types;

namespace Aurion::Vulkan
{
    Driver::Driver(
        const vk::raii::PhysicalDevice& physical_device,
        const LogicalDeviceConfig& device_config)
        : m_physical_device(physical_device), m_logical_device(nullptr)
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

        // TODO: Implement queue family aggregation and optimal selection.
        //      This can be done in 3 phases:
        //          1. Resolve each queue description to a specific queue family (opting for the one with the least flag bits set)
        //          2. Accumulate all queue descriptions per family
        //          3. Generate a single create info struct per family
        //      In the future, there needs to be a way to tie a specific queue(s) to their descriptions, and a handle for future use.
        {
            std::vector<vk::QueueFamilyProperties> qfprops = m_physical_device.getQueueFamilyProperties();
            std::vector<vk::DeviceQueueCreateInfo> create_queues{};

            for (u32 i = 0; i < qfprops.size(); ++i)
            {
                const auto& q = qfprops[i];
                AURION_WARN("Queue Flags for Queue [%d]: %d", i, q.queueFlags);
            }

            for (const auto& queue_desc : device_config.queues)
            {
                AURION_WARN("Looking For Flags: %d", queue_desc.flags);

                // Attempt to find
                auto qfp = std::ranges::find_if(
                    qfprops,
                    [&](const auto& prop)
                    {
                        return (prop.queueFlags & queue_desc.flags) != static_cast<vk::QueueFlags>(0);
                    });

                // If the element was found
                if (qfp != qfprops.end())
                {
                    // Grab its index
                    u32 index = std::distance(qfprops.begin(), qfp);

                    // Create the generation structure
                    vk::DeviceQueueCreateInfo dqcInfo;
                    dqcInfo.queueFamilyIndex = index;
                    dqcInfo.queueCount = queue_desc.count;
                    dqcInfo.pQueuePriorities = queue_desc.priorities.data();

                    // Add to queue creation data
                    create_queues.push_back(dqcInfo);
                }
            }
        }

        vk::DeviceCreateInfo dcInfo{};
        dcInfo.pNext = device_config.features;
        dcInfo.queueCreateInfoCount = static_cast<u32>(create_queues.size());
        dcInfo.pQueueCreateInfos = create_queues.data();
        dcInfo.enabledExtensionCount = static_cast<u32>(device_config.extensions.size());
        dcInfo.ppEnabledExtensionNames = device_config.extensions.data();

        // m_logical_device = vk::raii::Device(m_physical_device, dcInfo);
    }

    Driver::~Driver()
    {

    }

    void Driver::BeginFrame()
    {

    }

    void Driver::RecordCommands()
    {

    }

    void Driver::EndFrame()
    {

    }

    void Driver::CreateRenderTarget(const Window* window)
    {

    }
}
