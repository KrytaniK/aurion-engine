module;

#include <vulkan/vulkan_raii.hpp>
#include <string>
#include <vector>
#include <unordered_map>

export module Aurion.Vulkan:Shader;

import Aurion.Graphics;
import Aurion.FileSystem;

export namespace Aurion::Vulkan
{
    class Driver;

    class Shader : public Aurion::Shader
    {
    public:
        struct Config : Aurion::Shader::Config
        {
            std::vector<vk::VertexInputBindingDescription> vertex_bindings{};
            std::vector<vk::VertexInputAttributeDescription> vertex_attributes{};
            std::vector<vk::DescriptorSetLayoutBinding> descriptor_bindings{};
        };

    public:
        explicit Shader(const std::string_view& id);
        ~Shader() override;

        void Configure(const GraphicsInterface::Config* properties) override;
        void Attach(const IGraphicsDriver* driver) override;

        [[nodiscard]] const std::vector<EntryPoint>& GetEntryPoints() const;
        [[nodiscard]] const vk::raii::ShaderModule* GetModule(const Shader::EntryPoint& entry) const;

        [[nodiscard]] std::span<vk::VertexInputBindingDescription> GetVertexBindingDescriptions();
        [[nodiscard]] std::span<vk::VertexInputAttributeDescription> GetVertexAttributeDescriptions();
        [[nodiscard]] const vk::raii::DescriptorSetLayout& GetDescriptorSetLayout();

    protected:
        bool OnLoad() override;
        bool OnUnload() override;

    private:
        FSFileOpenParams GetFileAccessParameters() override;

    private:
        Config m_config;
        const Driver* m_driver;
        FSFile m_file_handle;
        std::unordered_map<Shader::Stage, vk::raii::ShaderModule> m_modules;
        vk::raii::DescriptorSetLayout m_desc_set_layout;
    };
}