module;

#include <vulkan/vulkan_raii.hpp>

export module Aurion.Vulkan:API;

import Aurion.Services;

import :Config;
import :Driver;

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

        const vk::raii::Context& GetContext() const;
        const vk::raii::Instance& GetInstance() const;

        // Creates a Vulkan driver instance based on the provided GPU and Logical Device properties, tied to this API instance.
        std::shared_ptr<Vulkan::Driver> CreateDriver(const PhysicalDeviceProperties& pDeviceProps, const DeviceProperties& device_props) const;

        // Attempts to select a suitable GPU for Vulkan operations.
        [[nodiscard]] vk::raii::PhysicalDevice GetPhysicalDevice(const PhysicalDeviceProperties& props) const;

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
