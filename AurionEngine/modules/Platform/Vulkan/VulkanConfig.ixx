module;

#include <vulkan/vulkan_raii.hpp>
#include <functional>

export module Aurion.Vulkan:Config;

import Aurion.Window;

import Aurion.Types;

export namespace Aurion::Vulkan
{
    // Optional GPU suitability function for advanced GPU property and feature requirements.
    typedef std::function<bool(const vk::raii::PhysicalDevice&)> GPUSuitabilityFn;

    // Handles basic Vulkan instance configuration
    struct APIConfig
    {
        vk::ApplicationInfo app_info{};
        const std::vector<const char*>& instance_extensions{};
        const std::vector<const char*>& instance_validation_layers{};
        bool validation_layers_enabled = false;
    };

    // Describes a request for one or more logical device queues
    struct QueueDescription
    {
        vk::QueueFlags flags{};
        u8 count = 0;
        std::vector<float> priorities{};
    };

    // Describes the desired property requirements for GPU selection
    struct PhysicalDeviceProperties
    {
        u32 min_api_version; // The minimum supported Vulkan API version
        vk::PhysicalDeviceType type; // The type of device (GPU/Integrated/CPU)
        vk::Flags<vk::QueueFlagBits> queue_flags{}; // Bit flags for desired rendering queues (graphics, compute, etc.)
        const std::vector<const char*>& extensions{}; // Required device extensions
        GPUSuitabilityFn gpu_suitability_fn = nullptr;
    };

    // Handles Vulkan Logical Device Configuration
    struct DeviceProperties
    {
        // A list of queue type preferences, paired with the desired queue creation amount and its priority
        const std::vector<QueueDescription> queues{};
        const std::vector<const char*>& extensions{}; // Required device extensions (if any)
        vk::PhysicalDeviceFeatures2* features = nullptr; // Required device features (if any)
    };

    const std::vector g_vk_default_validation_layers = {
        "VK_LAYER_KHRONOS_validation",
    };

    const std::vector g_vk_default_device_extensions = {
        vk::KHRSwapchainExtensionName,
    };

#ifdef NDEBUG
    constexpr bool g_vk_validation_layers_enabled = false;
#else
    constexpr bool g_vk_validation_layers_enabled = true;
#endif

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

    const PhysicalDeviceProperties g_vk_default_device_properties{
        // Target Vulkan 1.4
        .min_api_version = vk::ApiVersion14,
        // Prefer Dedicated GPU
        .type = vk::PhysicalDeviceType::eDiscreteGpu,
        // Support Graphics and Compute operations
        .queue_flags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute,
        //
        .extensions = g_vk_default_device_extensions
    };
}