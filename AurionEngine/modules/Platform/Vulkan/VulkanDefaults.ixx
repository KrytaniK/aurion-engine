module;

#include <vulkan/vulkan_raii.hpp>

export module Aurion.Vulkan:Defaults;

import :Config;

export namespace Aurion::Vulkan
{
#ifdef NDEBUG
    constexpr bool g_vk_validation_layers_enabled = false;
#else
    constexpr bool g_vk_validation_layers_enabled = true;
#endif

    const std::vector g_vk_default_validation_layers = {
        "VK_LAYER_KHRONOS_validation",
    };

    const std::vector g_vk_default_device_extensions = {
        vk::KHRSwapchainExtensionName,
    };

    constexpr vk::DebugUtilsMessageSeverityFlagsEXT g_vk_default_debug_message_severity_flags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    );

    constexpr vk::DebugUtilsMessageTypeFlagsEXT g_vk_default_debug_message_type_flags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding
    );

    constexpr PhysicalDeviceProperties g_vk_default_device_properties{
        // Target Vulkan 1.4
        .min_api_version = vk::ApiVersion14,
        // Prefer Dedicated GPU
        .type = vk::PhysicalDeviceType::eDiscreteGpu,
        // Support Graphics and Compute operations
        .queue_flags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute,
        //
        .extensions = g_vk_default_device_extensions,
    };
}