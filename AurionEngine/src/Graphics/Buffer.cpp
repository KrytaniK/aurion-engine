module;

#include <cstdint>
#include <string>
#include <memory>

module Aurion.Graphics;

namespace Aurion
{
    Buffer::Buffer(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const BufferDescription& desc)
        : m_driver(driver), m_handle({}), m_alias(id)
    {
        m_id = std::hash<std::string_view>()(id);

        // Promote GPU handle to a shared pointer
        m_handle = std::make_shared<BufferHandle>(m_driver->CreateBuffer(desc));
    }

    Buffer::~Buffer()
    {
        if (!m_driver || m_handle->value == 0 || m_handle.use_count() > 1) return;

        m_driver->Release(*m_handle);
    }

    GPUResourceType Buffer::GetType() const { return GPUResourceType::Buffer; }

    std::string_view Buffer::GetAlias() const { return m_alias; }

    const BufferHandle& Buffer::GetHandle() const { return *m_handle; }

    const u64& Buffer::GetID() const { return m_id; }

    bool Buffer::IsLoaded() const { return m_handle->value != 0; }
}
