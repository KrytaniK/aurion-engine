module;

#include <vulkan/vulkan_raii.hpp>
#include <string>

module Aurion.Vulkan;

// import Aurion.Graphics;

namespace Aurion::Vulkan
{
    Texture::Texture(const std::string_view& id)
        : Aurion::Texture(id), m_driver(nullptr), m_image(nullptr), m_view(nullptr)
    {

    }

    Texture::~Texture()
    {

    }

    Texture::Texture(Texture&& other) noexcept
        : Aurion::Texture(other.m_config.name), m_image(nullptr), m_view(nullptr)
    {
        m_config = std::move(other.m_config);
        m_driver = std::move(other.m_driver);
        m_image = std::move(other.m_image);
        m_image_memory = std::move(other.m_image_memory);
        m_view = std::move(other.m_view);
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this == &other) return *this;

        m_config = std::move(other.m_config);
        m_driver = std::move(other.m_driver);
        m_image = std::move(other.m_image);
        m_image_memory = std::move(other.m_image_memory);
        m_view = std::move(other.m_view);

        return *this;
    }

    void Texture::Configure(const GraphicsResource::Config* properties)
    {
        if (!m_driver) return;

        // Cache config structure
        m_config = *dynamic_cast<const Config*>(properties);

        // Generate image, if not already bound
        m_image = m_driver->AllocateImage(m_config);

        // Generate image view
        m_config.view.image = m_image;
        m_view = m_driver->AllocateImageView(m_image, m_config.view);
    }

    void Texture::Attach(const IGraphicsDriver* driver)
    {
        m_driver = dynamic_cast<const Driver*>(driver);
    }

    vk::MemoryRequirements Texture::GetMemoryRequirements() const
    {
        return m_image.getMemoryRequirements();
    }

    void Texture::BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset) const
    {
        m_image.bindMemory(*memory, offset);
    }

    u32 Texture::GetWidth() const
    {
        return m_config.width;
    }

    u32 Texture::GetHeight() const
    {
        return m_config.width;
    }

    const vk::raii::ImageView& Texture::GetView() const
    {
        return m_view;
    }

    vk::Format Texture::GetFormat() const
    {
        return m_config.image.format;
    }

    bool Texture::OnLoad()
    {
        return true;
    }

    bool Texture::OnUnload()
    {
        return true;
    }
}
