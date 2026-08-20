module;

#include <cstdint>
#include <string>
#include <memory>

module Aurion.Graphics;

namespace Aurion
{
    Shader::Shader(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const ShaderDescription& desc)
        : m_driver(driver), m_handle({}), m_alias(id)
    {
        m_id = std::hash<std::string_view>()(id);

        // Promote GPU handle to a shared pointer
        m_handle = std::make_shared<ShaderHandle>(m_driver->CreatePipeline(desc));
    }

    Shader::~Shader()
    {
        if (!m_driver || m_handle->value == 0 || m_handle.use_count() > 1) return;

        m_driver->Release(*m_handle);
    }

    GPUResourceType Shader::GetType() const { return GPUResourceType::Shader; }

    std::string_view Shader::GetAlias() const { return m_alias; }

    const ShaderHandle& Shader::GetHandle() const { return *m_handle; }

    const u64& Shader::GetID() const { return m_id; }

    bool Shader::IsLoaded() const { return m_handle->value != 0; }
}
