module;

#include <string>
#include <memory>

export module Aurion.Graphics:Sampler;

import :Interface;
import :Config;
import :Types;

export namespace Aurion
{
    class Sampler : IGraphicsAsset
    {
    public:
        explicit Sampler(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const SamplerDescription& desc);
        ~Sampler() override;

        [[nodiscard]] GPUResourceType GetType() const final;

        [[nodiscard]] std::string_view GetAlias() const final;

        [[nodiscard]] const SamplerHandle& GetHandle() const final;

        [[nodiscard]] const u64& GetID() const final;

        [[nodiscard]] bool IsLoaded() const final;

    private:
        std::shared_ptr<IGraphicsDriver> m_driver;
        std::shared_ptr<SamplerHandle> m_handle;
        std::string m_alias;
        u64 m_id;
    };
}