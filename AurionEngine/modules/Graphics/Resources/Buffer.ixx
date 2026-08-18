module;

#include <string>
#include <memory>

export module Aurion.Graphics:Buffer;

import Aurion.Resources;
import Aurion.Types;

import :Interface;
import :Config;
import :Types;

export namespace Aurion
{
    class Buffer : IGraphicsAsset
    {
    public:
        explicit Buffer(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const BufferDescription& desc);
        ~Buffer() override;

        [[nodiscard]] GPUResourceType GetType() const final;

        [[nodiscard]] std::string_view GetAlias() const final;

        [[nodiscard]] const BufferHandle& GetHandle() const final;

        [[nodiscard]] const u64& GetID() const final;

        [[nodiscard]] bool IsLoaded() const final;

    private:
        std::shared_ptr<IGraphicsDriver> m_driver;
        std::shared_ptr<PipelineHandle> m_handle;
        std::string m_alias;
        u64 m_id;
    };
}