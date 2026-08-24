module;

#include <AurionLog.h>
#include <vulkan/vulkan_raii.hpp>
#include <stdexcept>
#include <vector>

module Aurion.Vulkan;

import Aurion.Services;
import Aurion.Types;

namespace Aurion::Vulkan
{
    API::API(const APIConfig& config)
        : m_config(config), m_instance(nullptr), m_debug_messenger(nullptr)
    { }

    const vk::raii::Context& API::GetContext() const
    {
        return m_context;
    }

    const vk::raii::Instance& API::GetInstance() const
    {
        return m_instance;
    }

    std::shared_ptr<Driver> API::CreateDriver(const PhysicalDeviceProperties& pDeviceProps, const DeviceProperties& device_props) const
    {
        return std::make_shared<Driver>(this, pDeviceProps, device_props);
    }

    void API::ValidateInstanceExtensions() const
    {
        auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
        for (const auto& extension : m_config.instance_extensions)
        {
            if (std::ranges::none_of(
                extensionProperties,
                [ext = extension](const auto& ext_prop)
                {
                    return strcmp(ext_prop.extensionName, ext) == 0;
                }
            ))
            {
                throw std::runtime_error(
                    "[Vulkan API] Required Extension \"" + std::string(extension) + "\" not supported");
            }
        }
    }

    void API::ValidateValidationLayerCompatibility() const
    {
        if (!m_config.validation_layers_enabled) return;

        std::vector required(
            m_config.instance_validation_layers.begin(),
            m_config.instance_validation_layers.end()
        );

        auto layerProperties = m_context.enumerateInstanceLayerProperties();

        const auto unsupportedLayerIt = std::ranges::find_if(
            required,
            [&layerProperties](const auto& layer)
            {
                return std::ranges::none_of(
                    layerProperties,
                    [layer](const auto& prop)
                    {
                        return strcmp(prop.layerName, layer) == 0;
                    }
                );
            }
        );

        if (unsupportedLayerIt != required.end())
        {
            throw std::runtime_error(
                "[Vulkan API] Required Validation Layer \"" + std::string(*unsupportedLayerIt) + "\" not supported");
        }
    }

    vk::raii::PhysicalDevice API::GetPhysicalDevice(const PhysicalDeviceProperties& props) const
    {
        // Query for available GPUs
        auto pDevices = m_instance.enumeratePhysicalDevices();

        if (pDevices.empty())
            throw std::runtime_error("[Vulkan Driver] No physical device found on this system");

        bool meets_basic_reqs = false;

        vk::PhysicalDeviceProperties dProps{};

        // Search for a device that meets all requirements
        const auto it = std::ranges::find_if(pDevices, [&](const auto& device)
        {
            // Get each device's property/feature availability
            dProps = device.getProperties();

            // Find supported queue families and available device extensions
            auto queue_families = device.getQueueFamilyProperties();
            auto available_extensions = device.enumerateDeviceExtensionProperties();

            AURION_TRACE("[Vulkan] Device Found: %s", &dProps.deviceName);

            const bool meets_api_version = dProps.apiVersion >= props.min_api_version;
            const bool is_preferred_type = dProps.deviceType == props.type;
            const bool is_queue_compatible = std::ranges::any_of(queue_families, [&](const auto& qProps)
                {
                    return !!(qProps.queueFlags & props.queue_flags);
                });
            const bool has_all_extensions = std::ranges::all_of(props.extensions,
                [&](const auto& required_ext)
                {
                    return std::ranges::any_of(available_extensions,
                       [&required_ext](const auto& available_ext)
                       {
                           return strcmp(
                               available_ext.extensionName,
                               required_ext) == 0;
                       });
                });

            // Verify basic device requirements
            meets_basic_reqs =
                // Verify API version compatibility
                meets_api_version &&
                // Verify preferred GPU/CPU type
                is_preferred_type &&
                // Verify compatibility with specified queue families
                is_queue_compatible &&
                // Verify required extension compatibility
                has_all_extensions;

            // NOTE: Device feature compatibility should be checked in the suitability function.
            return meets_basic_reqs && (props.gpu_suitability_fn ? props.gpu_suitability_fn(device) : true);
        });

        return it == pDevices.end() ? vk::raii::PhysicalDevice(nullptr) : *it;
    }

    void API::OnRegister()
    {
        // Validate Required Instance Extensions
        ValidateInstanceExtensions();

        // Validate compatibility with Vulkan Validation Layers
        ValidateValidationLayerCompatibility();

        // Create Vulkan Instance
        vk::InstanceCreateInfo instance_create_info;
        instance_create_info.pApplicationInfo = &m_config.app_info;
        instance_create_info.enabledExtensionCount = static_cast<uint32_t>(m_config.instance_extensions.size());
        instance_create_info.enabledLayerCount = static_cast<uint32_t>(m_config.instance_validation_layers.size());
        instance_create_info.ppEnabledExtensionNames = m_config.instance_extensions.data();
        instance_create_info.ppEnabledLayerNames = m_config.instance_validation_layers.data();

        try
        {
            m_instance = vk::raii::Instance(m_context, instance_create_info);
        }
        catch (const vk::SystemError& e)
        {
            throw std::runtime_error(e.what());
        }

        // Set up Debug Messenger
        if (m_config.validation_layers_enabled)
        {
            vk::DebugUtilsMessengerCreateInfoEXT dbm_create_info{};
            dbm_create_info.messageSeverity = g_vk_default_debug_message_severity_flags;
            dbm_create_info.messageType = g_vk_default_debug_message_type_flags;
            dbm_create_info.pfnUserCallback = &DebugMessageCallback;

            m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(dbm_create_info);
        }
    }

    void API::OnRestart()
    {

    }

    void API::OnUnregister()
    {

    }

    vk::Bool32 DebugMessageCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        switch (severity)
        {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            {
                AURION_TRACE("[Vulkan Validation] \n\tType: %s\n\tMessage: %s", to_string(type).c_str(),
                             pCallbackData->pMessage);
                break;
            }
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            {
                AURION_INFO("[Vulkan Validation] \n\tType: %s\n\tMessage: %s", to_string(type).c_str(),
                            pCallbackData->pMessage);
                break;
            }
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            {
                AURION_WARN("[Vulkan Validation] \n\tType: %s\n\tMessage: %s", to_string(type).c_str(),
                            pCallbackData->pMessage);

                break;
            }
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            {
                AURION_ERROR("[Vulkan Validation] \n\tType: %s\n\tMessage: %s", to_string(type).c_str(),
                             pCallbackData->pMessage);
                break;
            }
        }

        return vk::False;
    }
}
