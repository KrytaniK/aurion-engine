module;

#include <AurionLog.h>
#include <vulkan/vulkan_raii.hpp>
#include <stdexcept>
#include <vector>
#include <span>

module Aurion.Vulkan;

import Aurion.Services;
import Aurion.Types;

namespace Aurion
{
    vk::Bool32 VulkanDebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        switch (severity)
        {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            {
                AURION_TRACE("[Vulkan Validation] \n\tType: %s\n\tMessage: %s", to_string(type), pCallbackData->pMessage);
                break;
            }
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            {
                AURION_INFO("[Vulkan Validation] \n\tType: %s\n\tMessage: %s", to_string(type), pCallbackData->pMessage);
                break;
            }
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            {
                AURION_WARN("[Vulkan Validation] \n\tType: %s\n\tMessage: %s", to_string(type), pCallbackData->pMessage);

                break;
            }
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            {
                AURION_ERROR("[Vulkan Validation] \n\tType: %s\n\tMessage: %s", to_string(type), pCallbackData->pMessage);
                break;
            }
        }

        return vk::False;
    }

    VulkanDriver::VulkanDriver(const VulkanDriverConfig& config)
        : m_instance(nullptr), m_debug_messenger(nullptr)
    {
        // Ensure access to application resources
        m_resource_manager = ServiceLocator::GetService<ResourceManager>();

        // Validate that required extensions exist
        ValidateExtensions(config.extensions);

        // Configure instance creation
        vk::InstanceCreateInfo create_info = vk::InstanceCreateInfo()
            .setPApplicationInfo(&config.app_info)
            .setEnabledExtensionCount(config.extension_count)
            .setPEnabledExtensionNames(config.extensions);

        // Check validation layer compatibility and optionally enable them (enabled via project build config)
        ToggleValidationLayers();

        // Attempt to create Vulkan Instance
        try
        {
            m_instance = vk::raii::Instance(m_context, create_info);
        }
        catch (const vk::SystemError& e)
        {
            throw std::runtime_error(e.what());
        }

        // Optionally enable debug messaging (enabled via project build config)
        ToggleDebugMessenger();
    }

    VulkanDriver::~VulkanDriver()
    {

    }

    void VulkanDriver::BeginFrame()
    {

    }

    void VulkanDriver::RecordCommands()
    {

    }

    void VulkanDriver::EndFrame()
    {

    }

    void VulkanDriver::CreateRenderTarget(const Window* window)
    {

    }

    void VulkanDriver::ValidateExtensions(const std::span<const char*>& extensions) const
    {
        auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
        for (const auto& extension : extensions)
        {
            if (std::ranges::none_of(
                    extensionProperties,
                    [ext = extension](const auto& ext_prop)
                    {
                        return strcmp(ext_prop.extensionName, ext) == 0;
                    }
                ))
            {
                throw std::runtime_error("Required Extension \"" + std::string(extension) + "\" not supported");
            }
        }
    }

    void VulkanDriver::ToggleValidationLayers() const
    {
        if constexpr (!g_vk_validation_layers_enabled) return;

        std::vector required(g_vk_validation_layers.begin(), g_vk_validation_layers.end());

        auto layerProperties = m_context.enumerateInstanceLayerProperties();

        auto unsupportedLayerIt = std::ranges::find_if(
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
            throw std::runtime_error("Required Validation Layer \"" + std::string(*unsupportedLayerIt) + "\" not supported");
        }
    }

    void VulkanDriver::ToggleDebugMessenger()
    {
        // TODO (FUTURE): Make this configurable!

        if constexpr (!g_vk_validation_layers_enabled) return;

        // Set desired severity flags (All enabled by default
        const vk::DebugUtilsMessageSeverityFlagsEXT severity_flags(
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        );

        const vk::DebugUtilsMessageTypeFlagsEXT type_flags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding
        );

        vk::DebugUtilsMessengerCreateInfoEXT create_info;
        create_info
            .setMessageSeverity(severity_flags)
            .setMessageType(type_flags)
            .setPfnUserCallback(&VulkanDebugCallback);


        m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(create_info);
    }
}
