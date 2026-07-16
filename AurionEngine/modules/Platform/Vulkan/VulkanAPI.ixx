module;

#include <vulkan/vulkan_raii.hpp>

export module Aurion.Vulkan:API;

import Aurion.Services;

import :Config;

export namespace Aurion::Vulkan
{
    // Debug Messenger Callback for Vulkan Validation Layers
    VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugMessageCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    // A Service class wrapping Vulkan initialization and application-level objects/methods,
    // such as the Vulkan instance object and relevant context.
    class API : public IService
    {
    public:
        explicit API(const APIConfig& config);
        ~API() override = default;

        const vk::raii::Context& GetContext();
        const vk::raii::Instance& GetInstance();

        // Attempts to select a suitable GPU for Vulkan operations based on
        // the provided arguments, or the defaults if no arguments are specified.
        vk::raii::PhysicalDevice GetPhysicalDevice(
            const PhysicalDeviceProperties* prop_reqs = nullptr,
            const PhysicalDeviceSuitabilityFn& suitability_fn = nullptr
        ) const;

    private:
        void OnRegister() override;
        void OnRestart() override;
        void OnUnregister() override;

        void ValidateInstanceExtensions() const;
        void ValidateValidationLayerCompatibility() const;

    private:
        const APIConfig m_config;
        vk::raii::Context m_context;
        vk::raii::Instance m_instance;
        vk::raii::DebugUtilsMessengerEXT m_debug_messenger;
    };
}
