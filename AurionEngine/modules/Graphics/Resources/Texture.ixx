module;

#include <string>
#include <memory>

export module Aurion.Graphics:Texture;

import :Interface;
import :Config;
import :Types;

export namespace Aurion
{
    class Texture : IGraphicsAsset
    {
    public:
        explicit Texture(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const TextureDescription& desc);
        ~Texture() override;

        [[nodiscard]] GPUResourceType GetType() const final;

        [[nodiscard]] std::string_view GetAlias() const final;

        [[nodiscard]] const TextureHandle& GetHandle() const final;

        [[nodiscard]] const u64& GetID() const final;

        [[nodiscard]] bool IsLoaded() const final;

    private:
        std::shared_ptr<IGraphicsDriver> m_driver;
        std::shared_ptr<TextureHandle> m_handle;
        std::string m_alias;
        u64 m_id;
    };
}