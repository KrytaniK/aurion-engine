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

    class RenderTarget : public Aurion::RenderTarget
    {
    public:
        struct Config : Aurion::RenderTarget::Config
        {
            vk::ImageCreateInfo image = vk::ImageCreateInfo{}
                .setFormat(vk::Format::eB8G8R8A8Srgb)
                .setUsage(vk::ImageUsageFlagBits::eColorAttachment)
                .setSharingMode(vk::SharingMode::eExclusive);
            vk::ImageLayout image_finalLayout = vk::ImageLayout::eReadOnlyOptimal;

            vk::ImageViewCreateInfo view = vk::ImageViewCreateInfo{}
                .setViewType(vk::ImageViewType::e2D)
                .setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 })
                .setComponents({
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                });

            // Presentation (Disregarded if not binding to a window/surface)
            vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
            vk::ColorSpaceKHR color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
            vk::CompositeAlphaFlagBitsKHR composite_alpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            bool clipped = true;
        };

    public:
        explicit RenderTarget(const std::string_view& id);
        ~RenderTarget() override = default;

        void Configure(const Aurion::RenderTarget::Config* properties) override;

        void Attach(Window* window) override;

        void Validate() override;

        [[nodiscard]] u32 GetWidth() const override;
        [[nodiscard]] u32 GetHeight() const override;

        [[nodiscard]] const vk::Image& GetImage() const;
        [[nodiscard]] const vk::raii::ImageView& GetView() const;

        void AssignToDriver(const Driver* driver);

    protected:
        bool OnLoad() override;
        bool OnUnload() override;

    private:
        Config m_config;
        const Driver* m_driver{};
        u32 m_current_index{};
        std::vector<vk::Image> m_images;
        std::vector<vk::raii::ImageView> m_views;
        Window* m_window;
        vk::raii::SurfaceKHR m_surface; // Optional VkSurfaceKHR; Used when attaching to a window
        vk::raii::SwapchainKHR m_swapchain; // Optional swapchain; Used when attaching to a window
    };
}