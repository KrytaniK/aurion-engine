module;

#include <vulkan/vulkan_raii.hpp>
#include <string>
#include <vector>

export module Aurion.Vulkan:RenderTarget;

import Aurion.Graphics;
import Aurion.Types;

import :Config;

export namespace Aurion::Vulkan
{
    class Driver;

    // Configuration properties for this render target.
    //  Defaults to a 2D color attachment image in the SRGB non-linear color space.
    //  No sharing. vSync Enabled. 3 frames in flight. Identity Component Swizzle
    struct RenderTargetProperties : public Aurion::RenderTargetProperties
    {
        vk::Format format = vk::Format::eB8G8R8A8Srgb;
        vk::ColorSpaceKHR color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
        vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
        vk::ImageUsageFlags usage_flags = vk::ImageUsageFlagBits::eColorAttachment;
        vk::SharingMode share_mode = vk::SharingMode::eExclusive;
        vk::CompositeAlphaFlagBitsKHR composite_alpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        vk::ImageViewType view_type = vk::ImageViewType::e2D;
        vk::ImageSubresourceRange subresource_range = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
        vk::ComponentMapping components = {
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
        };
        bool clipped = true;
    };

    class RenderTarget : public Aurion::RenderTarget
    {
    public:
        explicit RenderTarget(const std::string_view& id);
        ~RenderTarget() override = default;

        void AssignToDriver(const Driver* driver);

        void Configure(const Aurion::RenderTargetProperties* properties) override;

        void Attach(Window* window) override;

        void Validate() override;

    protected:
        bool OnLoad() override;
        bool OnUnload() override;

    private:
        RenderTargetProperties m_config;
        const Driver* m_driver{};
        u32 m_current_index{};
        std::vector<vk::Image> m_images;
        std::vector<vk::raii::ImageView> m_views;
        Window* m_window;
        vk::raii::SurfaceKHR m_surface; // Optional VkSurfaceKHR; Used when attaching to a window
        vk::raii::SwapchainKHR m_swapchain; // Optional swapchain; Used when attaching to a window
    };
}