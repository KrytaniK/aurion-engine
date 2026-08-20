module;

#include <cstdint>
#include <string>
#include <memory>

module Aurion.Graphics;

namespace Aurion
{
    Sampler::Sampler(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const SamplerDescription& desc)
        : m_driver(driver), m_handle({}), m_alias(id)
    {
        m_id = std::hash<std::string_view>()(id);

        // Promote GPU handle to a shared pointer
        m_handle = std::make_shared<SamplerHandle>(m_driver->CreateSampler(desc));
    }

    Sampler::~Sampler()
    {
        if (!m_driver || m_handle->value == 0 || m_handle.use_count() > 1) return;

        m_driver->Release(*m_handle);
    }

    GPUResourceType Sampler::GetType() const { return GPUResourceType::Sampler; }

    std::string_view Sampler::GetAlias() const { return m_alias; }

    const SamplerHandle& Sampler::GetHandle() const { return *m_handle; }

    const u64& Sampler::GetID() const { return m_id; }

    bool Sampler::IsLoaded() const { return m_handle->value != 0; }
}
