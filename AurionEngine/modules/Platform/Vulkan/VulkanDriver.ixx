module;

#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include <span>

export module Aurion.Vulkan:Driver;

import Aurion.Graphics;
import Aurion.Resources;
import Aurion.Types;

export const std::vector<const char*> g_vk_validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    export constexpr bool g_vk_validation_layers_enabled = false;
#else
    export constexpr bool g_vk_validation_layers_enabled = true;
#endif

export namespace Aurion
{
    // Debug Messenger Callback for Vulkan Validation Layers
    VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanDebugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
        vk::DebugUtilsMessageTypeFlagsEXT              type,
        const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
        void *                                         pUserData);

    struct VulkanDriverConfig
    {
        vk::ApplicationInfo app_info; // General information about the application
        const std::span<const char*> extensions; // Extensions for interfacing with external APIs (Such as GLFW)
        u32 extension_count; // The number of instance extensions being used
    };

    class VulkanDriver : public IGraphicsDriver
    {
    public:
        explicit VulkanDriver(const VulkanDriverConfig& config);
        ~VulkanDriver() override;

        void BeginFrame() override;
        void RecordCommands() override;
        void EndFrame() override;

        void CreateRenderTarget(const Window* window) override;

    private:
        // Checks to ensure supplied instance extensions are supported by the
        //  installed Vulkan SDK
        void ValidateExtensions(const std::span<const char*>& extensions) const;

        // Enables Vulkan validation layers, if enabled
        void ToggleValidationLayers() const;

        // Setup Debug Messaging
        void ToggleDebugMessenger();

    private:
        ResourceManager* m_resource_manager;
        vk::raii::Context m_context;
        vk::raii::Instance m_instance;
        vk::raii::DebugUtilsMessengerEXT m_debug_messenger;
    };
}
