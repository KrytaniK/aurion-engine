module;

#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include <span>
#include <functional>

export module Aurion.Vulkan:Driver;

import Aurion.Graphics;
import Aurion.Resources;
import Aurion.Types;

import :Config;

export namespace Aurion::Vulkan
{

    class Driver : public IGraphicsDriver
    {
    public:
        explicit Driver(const vk::raii::PhysicalDevice& physical_device, const LogicalDeviceConfig& device_config);
        ~Driver() override;

        void BeginFrame() override;
        void RecordCommands() override;
        void EndFrame() override;

        void CreateRenderTarget(const Window* window) override;

    private:
        ResourceManager* m_resource_manager;
        const vk::raii::Context* m_context;
        const vk::raii::Instance* m_instance;
        vk::raii::PhysicalDevice m_physical_device;
        vk::raii::Device m_logical_device;
    };
}
