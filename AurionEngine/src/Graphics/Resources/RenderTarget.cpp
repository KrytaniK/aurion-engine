module;

#include <cstdint>
#include <string>
#include <memory>

module Aurion.Graphics;

namespace Aurion
{
    RenderTarget::RenderTarget(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const RenderTargetDescription& desc)
        : m_driver(driver), m_handle({}), m_alias(id)
    {
        m_id = std::hash<std::string_view>()(id);

        // Promote GPU handle to a shared pointer
        m_handle = std::make_shared<RenderTargetHandle>(m_driver->CreateRenderTarget(desc));
    }

    RenderTarget::~RenderTarget()
    {
        if (!m_driver || m_handle->value == 0 || m_handle.use_count() > 1) return;

        m_driver->Release(*m_handle);
    }

    GPUResourceType RenderTarget::GetType() const { return GPUResourceType::RenderTarget; }

    std::string_view RenderTarget::GetAlias() const { return m_alias; }

    const RenderTargetHandle& RenderTarget::GetHandle() const { return *m_handle; }

    const u64& RenderTarget::GetID() const { return m_id; }

    bool RenderTarget::IsLoaded() const { return m_handle->value != 0; }
}
