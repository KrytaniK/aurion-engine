module;

#include <cstdint>
#include <string>
#include <memory>

module Aurion.Graphics;

namespace Aurion
{
    Texture::Texture(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const TextureDescription& desc)
        : m_driver(driver), m_handle({}), m_alias(id)
    {
        m_id = std::hash<std::string_view>()(id);

        // Promote GPU handle to a shared pointer
        m_handle = std::make_shared<TextureHandle>(m_driver->CreateTexture(desc));
    }

    Texture::~Texture()
    {
        if (!m_driver || m_handle->value == 0 || m_handle.use_count() > 1) return;

        m_driver->Release(*m_handle);
    }

    GPUResourceType Texture::GetType() const { return GPUResourceType::Texture; }

    std::string_view Texture::GetAlias() const { return m_alias; }

    const TextureHandle& Texture::GetHandle() const { return *m_handle; }

    const u64& Texture::GetID() const { return m_id; }

    bool Texture::IsLoaded() const { return m_handle->value != 0; }
}
