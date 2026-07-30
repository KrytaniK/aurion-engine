module;

#include <vulkan/vulkan_raii.hpp>
#include <string>
#include <vector>
#include <memory>

export module Aurion.Vulkan:RenderTarget;

import Aurion.Graphics;
import Aurion.Types;

import :Config;
import :Texture;

export namespace Aurion::Vulkan
{
    class Driver;

    class IRenderTargetImpl
    {
    public:
        virtual ~IRenderTargetImpl() = default;

        virtual void Configure(const Driver* driver, const GraphicsResource::Config* properties) = 0;

        [[nodiscard]] virtual u32 GetWidth() const = 0;
        [[nodiscard]] virtual u32 GetHeight() const = 0;
        [[nodiscard]] virtual u32 GetDepth() const = 0;

        [[nodiscard]] virtual vk::MemoryRequirements GetMemoryRequirements() const = 0;
        virtual void BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset, const u32& index) const = 0;

        [[nodiscard]] virtual const vk::raii::ImageView& GetView(const u32& index) const = 0;

        [[nodiscard]] virtual vk::Format GetFormat() const = 0;

        virtual void SwapBuffers(u32& index) = 0;
    };

    class RenderTarget : public Aurion::RenderTarget
    {
        enum Type { Default = 0, Swapchain };

    public:
        struct _Config : Aurion::RenderTarget::Config
        {
            explicit _Config(const Type& type) : Aurion::RenderTarget::Config(), rtType(type) {};
            Type rtType = Default;
        };

        struct SwapchainConfig : RenderTarget::_Config
        {
            SwapchainConfig() : RenderTarget::_Config(Swapchain) {};

            Window* window = nullptr;
            vk::Format format = vk::Format::eUndefined;
            vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
            vk::SharingMode sharing_mode = vk::SharingMode::eExclusive;
            vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
            vk::ColorSpaceKHR color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
            vk::CompositeAlphaFlagBitsKHR composite_alpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            vk::ImageViewCreateInfo view_config = vk::ImageViewCreateInfo{}
                .setViewType(vk::ImageViewType::e2D)
                .setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 })
                .setComponents({
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                });
            bool clipped = true;
        };

        struct Config : RenderTarget::_Config
        {
            Config() : RenderTarget::_Config(Default) {};

            u32 width = 0;
            u32 height = 0;
            u32 depth = 0;
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
        };

    public:
        explicit RenderTarget(const std::string_view& id);
        ~RenderTarget() override = default;

        void Configure(const GraphicsResource::Config* properties) override;
        void Attach(const IGraphicsDriver* driver) override;

        void SwapBuffers() override;

        [[nodiscard]] u32 GetWidth() const override;
        [[nodiscard]] u32 GetHeight() const override;
        [[nodiscard]] u32 GetDepth() const override;

        [[nodiscard]] vk::MemoryRequirements GetMemoryRequirements() const;
        void BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset, const u32& index = 0) const;

        [[nodiscard]] const vk::raii::ImageView& GetView() const;

        [[nodiscard]] vk::Format GetFormat() const;

    protected:
        bool OnLoad() override;
        bool OnUnload() override;

    private:
        const Driver* m_driver;
        std::unique_ptr<IRenderTargetImpl> m_impl;
        u32 m_buffer_index;
    };

    class RenderTargetImpl_Swapchain : public IRenderTargetImpl
    {
    public:
        RenderTargetImpl_Swapchain();
        ~RenderTargetImpl_Swapchain() override = default;

        void Configure(const Driver* driver, const GraphicsResource::Config* properties) override;

        [[nodiscard]] u32 GetWidth() const override;
        [[nodiscard]] u32 GetHeight() const override;
        [[nodiscard]] u32 GetDepth() const override;

        [[nodiscard]] vk::MemoryRequirements GetMemoryRequirements() const override;
        void BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset, const u32& index) const override;

        [[nodiscard]] const vk::raii::ImageView& GetView(const u32& index) const override;

        [[nodiscard]] vk::Format GetFormat() const override;

        void SwapBuffers(u32& index) override;

    private:
        RenderTarget::SwapchainConfig m_config;
        const WindowProperties* m_win_props;
        const Driver* m_driver;
        vk::raii::SurfaceKHR m_surface;
        vk::raii::SwapchainKHR m_swapchain;
        std::vector<vk::Image> m_images;
        std::vector<vk::raii::ImageView> m_views;
    };

    class RenderTargetImpl_Default : public IRenderTargetImpl
    {
    public:
        RenderTargetImpl_Default();
        ~RenderTargetImpl_Default() override = default;

        void Configure(const Driver* driver, const GraphicsResource::Config* properties) override;

        [[nodiscard]] u32 GetWidth() const override;
        [[nodiscard]] u32 GetHeight() const override;
        [[nodiscard]] u32 GetDepth() const override;

        [[nodiscard]] vk::MemoryRequirements GetMemoryRequirements() const override;
        void BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset, const u32& index) const override;

        [[nodiscard]] const vk::raii::ImageView& GetView(const u32& index) const override;

        [[nodiscard]] vk::Format GetFormat() const override;

        void SwapBuffers(u32& index) override;

    private:
        RenderTarget::Config m_config;
        const Driver* m_driver;
        std::vector<Vulkan::Texture> m_targets;
    };
}