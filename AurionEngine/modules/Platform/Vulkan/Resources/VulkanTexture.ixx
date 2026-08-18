module;

#include <vulkan/vulkan_raii.hpp>

export module Aurion.Vulkan:Texture;

import Aurion.Graphics;
import Aurion.Types;

export namespace Aurion::Vulkan
{
    class Driver;
    class Texture : public Aurion::Texture
    {
    public:
        struct Config : Aurion::Texture::Config
        {
            vk::ImageCreateInfo image = vk::ImageCreateInfo{}
                .setImageType(vk::ImageType::e2D)
                .setExtent(vk::Extent3D(0, 0, 1))
                .setMipLevels(1)
                .setArrayLayers(1)
                .setTiling(vk::ImageTiling::eOptimal)
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
        explicit Texture(const std::string_view& id);
        ~Texture() override;

        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        void Configure(const GraphicsInterface::Config* properties) override;
        void Attach(const IGraphicsDriver* driver) override;

        [[nodiscard]] vk::MemoryRequirements GetMemoryRequirements() const;
        void BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset) const;

        [[nodiscard]] u32 GetWidth() const override;
        [[nodiscard]] u32 GetHeight() const override;

        [[nodiscard]] const vk::raii::Image& GetImage() const;
        [[nodiscard]] const vk::raii::ImageView& GetView() const;

        [[nodiscard]] vk::Format GetFormat() const;

    protected:
        bool OnLoad() override;
        bool OnUnload() override;

    private:
        Config m_config;
        const Driver* m_driver;
        vk::raii::Image m_image;
        vk::raii::ImageView m_view;
        std::shared_ptr<vk::raii::DeviceMemory> m_image_memory;
    };
}
