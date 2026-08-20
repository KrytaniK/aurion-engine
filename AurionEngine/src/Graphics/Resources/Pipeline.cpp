module;

#include <cstdint>
#include <string>
#include <memory>

module Aurion.Graphics;

namespace Aurion
{
    Pipeline::Pipeline(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const PipelineDescription& desc)
        : m_driver(driver), m_handle({}), m_alias(id)
    {
        m_id = std::hash<std::string_view>()(id);

        // Promote GPU handle to a shared pointer so copies don't release the resource on destruction
        m_handle = std::make_shared<PipelineHandle>(m_driver->CreatePipeline(desc));
    }

    Pipeline::~Pipeline()
    {
        if (!m_driver || m_handle->value == 0 || m_handle.use_count() > 1) return;

        m_driver->Release(*m_handle);
    }

    GPUResourceType Pipeline::GetType() const { return GPUResourceType::Pipeline; }

    std::string_view Pipeline::GetAlias() const { return m_alias; }

    const PipelineHandle& Pipeline::GetHandle() const { return *m_handle; }

    const u64& Pipeline::GetID() const { return m_id; }

    bool Pipeline::IsLoaded() const { return m_handle->value != 0; }
}
