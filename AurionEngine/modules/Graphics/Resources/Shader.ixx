module;

#include <memory>
#include <string>

export module Aurion.Graphics:Shader;

import Aurion.Types;

import Aurion.FileSystem;

import :Interface;
import :Config;
import :Types;

export namespace Aurion
{
    class Shader : IGraphicsAsset
    {
    public:
        explicit Shader(const std::string_view& id, const std::shared_ptr<IGraphicsDriver>& driver, const ShaderDescription& desc);
        ~Shader() override;

        [[nodiscard]] GPUResourceType GetType() const final;

        [[nodiscard]] std::string_view GetAlias() const final;

        [[nodiscard]] const ShaderHandle& GetHandle() const final;

        [[nodiscard]] const u64& GetID() const final;

        [[nodiscard]] bool IsLoaded() const final;

    private:
        std::shared_ptr<IGraphicsDriver> m_driver;
        std::shared_ptr<ShaderHandle> m_handle;
        std::string m_alias;
        u64 m_id;
    };
}