module;

#include <vulkan/vulkan_raii.hpp>
#include <string>

export module Aurion.Vulkan:Shader;

import Aurion.Graphics;
import Aurion.FileSystem;

import :Driver;

export namespace Aurion::Vulkan
{
    class Shader : public Aurion::Shader
    {
    public:
        explicit Shader(const std::string_view& id);
        ~Shader() override;

        void Configure(const GraphicsResource::Config* properties) override;
        void Attach(const IGraphicsDriver* driver) override;

        [[nodiscard]] const std::vector<EntryPoint>& GetEntryPoints() const;
        [[nodiscard]] const vk::raii::ShaderModule& GetModule() const;

    protected:
        bool OnLoad() override;
        bool OnUnload() override;

    private:
        FSFileOpenParams GetFileAccessParameters() override;

    private:
        Config m_config;
        const Driver* m_driver;
        FSFile m_file_handle;
        vk::raii::ShaderModule m_module;
    };
}