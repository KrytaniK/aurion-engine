module;

#include <string>
#include <cstring>

module Aurion.Vulkan;

namespace Aurion::Vulkan
{
    Buffer::Buffer(const std::string_view& id)
        : Aurion::Buffer(id), m_buffer(nullptr), m_buffer_memory(nullptr)
    {

    }

    void Buffer::Configure(const GraphicsResource::Config* properties)
    {
        // Save local configuration
        m_config = *dynamic_cast<const Config*>(properties);

        // Generate the buffer object
        m_buffer = m_driver->AllocateBuffer(m_config);
        // Allocate buffer memory
        m_buffer_memory = m_driver->AllocateBufferMemory(m_buffer, m_config.properties);

        // Bind the buffer to the allocated memory
        m_buffer.bindMemory(m_buffer_memory, 0);
    }

    void Buffer::Attach(const IGraphicsDriver* driver)
    {
        m_driver = static_cast<const Driver*>(driver);
    }

    void Buffer::Write(void* data, const u32& offset, const u32& size)
    {
        void* mapped = m_buffer_memory.mapMemory(offset, m_config.size);
        std::memcpy(mapped, data, m_config.size);
        m_buffer_memory.unmapMemory();
    }

    u32 Buffer::GetSize() const
    {
        return m_config.size;
    }

    bool Buffer::OnLoad()
    {
        return true;
    }

    bool Buffer::OnUnload()
    {
        return true;
    }
}
