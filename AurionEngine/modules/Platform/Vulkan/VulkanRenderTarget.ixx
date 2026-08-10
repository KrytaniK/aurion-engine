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

        [[nodiscard]] virtual const vk::Image& GetImage(const u32& index) const = 0;
        [[nodiscard]] virtual const vk::raii::ImageView& GetView(const u32& index) const = 0;

        [[nodiscard]] virtual vk::Format GetFormat() const = 0;

        [[nodiscard]] virtual const vk::raii::Semaphore* GetWaitSemaphore(const u32& index) const = 0;
        [[nodiscard]] virtual const vk::raii::Semaphore* GetSignalSemaphore(const u32& index) const = 0;

        virtual void SwapBuffers(const u32& frame_index, u32& image_index) = 0;

        [[nodiscard]] virtual bool CanPresent() const = 0;

        virtual void Present(const vk::raii::Queue& queue, const u32& buffer_index) = 0;
    };

    class RenderTarget : public Aurion::RenderTarget
    {
        enum Type { Default = 0, Swapchain };

        struct ConfigBase : Aurion::RenderTarget::Config
        {
            explicit ConfigBase(const Type& type) : Aurion::RenderTarget::Config(), rtType(type) {};
            Type rtType = Default;
        };

    public:
        struct SwapchainConfig : RenderTarget::ConfigBase
        {
            SwapchainConfig() : RenderTarget::ConfigBase(Swapchain) {};

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

        struct Config : RenderTarget::ConfigBase
        {
            Config() : RenderTarget::ConfigBase(Default) {};

            Vulkan::Texture::Config texture;
        };

    public:
        explicit RenderTarget(const std::string_view& id);
        ~RenderTarget() override;

        void Configure(const GraphicsResource::Config* properties) override;
        void Attach(const IGraphicsDriver* driver) override;

        // Swap internal image buffers for this target, and return the new index
        u32 SwapBuffers(const u32& frame_index) override;

        [[nodiscard]] u32 GetWidth() const override;
        [[nodiscard]] u32 GetHeight() const override;
        [[nodiscard]] u32 GetDepth() const override;

        [[nodiscard]] vk::MemoryRequirements GetMemoryRequirements() const;
        void BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset, const u32& index = 0) const;

        [[nodiscard]] const vk::Image& GetImage() const;
        [[nodiscard]] const vk::raii::ImageView& GetView() const;

        [[nodiscard]] vk::Format GetFormat() const;

        [[nodiscard]] const vk::raii::Semaphore* GetWaitSemaphore(const u32& index) const;
        [[nodiscard]] const vk::raii::Semaphore* GetSignalSemaphore(const u32& index) const;

        [[nodiscard]] bool CanPresent() const;

        void Present(const vk::raii::Queue& queue) const;

    protected: // Methods
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
        ~RenderTargetImpl_Swapchain() override;

        void Configure(const Driver* driver, const GraphicsResource::Config* properties) override;

        [[nodiscard]] u32 GetWidth() const override;
        [[nodiscard]] u32 GetHeight() const override;
        [[nodiscard]] u32 GetDepth() const override;

        [[nodiscard]] vk::MemoryRequirements GetMemoryRequirements() const override;
        void BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset, const u32& index) const override;

        [[nodiscard]] const vk::Image& GetImage(const u32& index) const override;
        [[nodiscard]] const vk::raii::ImageView& GetView(const u32& index) const override;

        [[nodiscard]] vk::Format GetFormat() const override;

        [[nodiscard]] const vk::raii::Semaphore* GetWaitSemaphore(const u32& index) const override;
        [[nodiscard]] const vk::raii::Semaphore* GetSignalSemaphore(const u32& index) const override;

        void SwapBuffers(const u32& frame_index, u32& image_index) override;

        [[nodiscard]] bool CanPresent() const override;

        void Present(const vk::raii::Queue& queue, const u32& buffer_index) override;

    private:
        void RecreateSwapchain();

    private:
        RenderTarget::SwapchainConfig m_config;
        const WindowProperties* m_win_props;
        const Driver* m_driver;
        vk::raii::SurfaceKHR m_surface;
        vk::raii::SwapchainKHR m_swapchain;
        std::vector<vk::Image> m_images;
        std::vector<vk::raii::ImageView> m_views;
        std::vector<vk::raii::Semaphore> m_wait_semaphores;
        std::vector<vk::raii::Semaphore> m_signal_semaphores;
        const vk::raii::Queue* m_present_queue;
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

        [[nodiscard]] const vk::Image& GetImage(const u32& index) const override;
        [[nodiscard]] const vk::raii::ImageView& GetView(const u32& index) const override;

        [[nodiscard]] vk::Format GetFormat() const override;

        [[nodiscard]] const vk::raii::Semaphore* GetWaitSemaphore(const u32& index) const override;
        [[nodiscard]] const vk::raii::Semaphore* GetSignalSemaphore(const u32& index) const override;

        void SwapBuffers(const u32& frame_index, u32& image_index) override;

        [[nodiscard]] bool CanPresent() const override;

        void Present(const vk::raii::Queue& queue, const u32& buffer_index) override;

    private:
        RenderTarget::Config m_config;
        const Driver* m_driver;
        std::vector<Vulkan::Texture> m_targets;
    };
}
