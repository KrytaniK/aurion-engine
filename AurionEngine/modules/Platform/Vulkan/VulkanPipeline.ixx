module;

#include <vulkan/vulkan_raii.hpp>
#include <string>

export module Aurion.Vulkan:Pipeline;

import Aurion.Resources;
import Aurion.Graphics;
import Aurion.Types;

export namespace Aurion::Vulkan
{
    class Driver;

    class Pipeline : public Aurion::Pipeline
    {
    public:
        struct Config : public Aurion::Pipeline::Config
        {
            explicit Config(const Aurion::Pipeline::Type& type) : Aurion::Pipeline::Config(type) {};

            std::vector<std::string> shaders{}; // Shader references by name
            vk::PipelineLayoutCreateInfo layout_info{};
            vk::AllocationCallbacks alloc_callbacks = nullptr;
            bool use_cache = false; // If enabled, uses a vk::raii::PipelineCache for allocation
        };

    public:
        explicit Pipeline(const std::string_view& id)
            : Aurion::Pipeline(id), m_driver(nullptr), m_pipeline(nullptr), m_layout(nullptr) {};
        ~Pipeline() override = default;

        void Configure(const GraphicsResource::Config* properties) override = 0;

        void Attach(const IGraphicsDriver* driver) override = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;

    protected:
        const Driver* m_driver{};
        vk::raii::Pipeline m_pipeline;
        vk::raii::PipelineLayout m_layout;
    };

    class GraphicsPipeline : public Pipeline
    {
    public:
        struct Config : public Pipeline::Config, public vk::GraphicsPipelineCreateInfo
        {
            explicit Config()
                : Pipeline::Config(Aurion::Pipeline::Type::Graphics)
            {};
        };

    public:
        explicit GraphicsPipeline(const std::string_view& id);
        ~GraphicsPipeline() override;

        void Configure(const GraphicsResource::Config* properties) override;
        void Attach(const IGraphicsDriver* driver) override;

    protected:
        bool OnLoad() override;
        bool OnUnload() override;

    private:
        Config m_config;
        const Driver* m_driver;
        ResourceManager* m_resource_manager;
        std::vector<ResourceHandle<Aurion::Shader>> m_shader_handles;
    };
}