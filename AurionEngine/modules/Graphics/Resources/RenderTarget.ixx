module;

#include <string>
#include <memory>

export module Aurion.Graphics:RenderTarget;

import :Interface;
import :Config;
import :Types;

export namespace Aurion
{
    class RenderTarget : IGraphicsAsset
    {
    public:
        explicit RenderTarget(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const RenderTargetDescription& desc);
        ~RenderTarget() override;

        [[nodiscard]] GPUResourceType GetType() const final;

        [[nodiscard]] std::string_view GetAlias() const final;

        [[nodiscard]] const RenderTargetHandle& GetHandle() const final;

        [[nodiscard]] const u64& GetID() const final;

        [[nodiscard]] bool IsLoaded() const final;

    private:
        std::shared_ptr<IGraphicsDriver> m_driver;
        std::shared_ptr<PipelineHandle> m_handle;
        std::string m_alias;
        u64 m_id;
    };
}