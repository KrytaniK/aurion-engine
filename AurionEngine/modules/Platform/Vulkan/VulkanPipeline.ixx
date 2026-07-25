module;

#include <vulkan/vulkan_raii.hpp>
#include <string>

export module Aurion.Vulkan:Pipeline;

import Aurion.Graphics;
import Aurion.Types;

import :Driver;

export namespace Aurion::Vulkan
{
    class Pipeline : public Aurion::Pipeline
    {
    public:
        struct Config : public Aurion::Pipeline::Config
        {
            VkPipelineCreateFlags flags;
            vk::AllocationCallbacks alloc_callbacks = nullptr;
            bool use_cache = false; // If enabled, uses a vk::raii::PipelineCache for allocation
        };

    public:
        explicit Pipeline(const std::string_view& id)
            : Aurion::Pipeline(id), m_driver(nullptr), m_pipeline(nullptr), m_layout(nullptr) {};
        ~Pipeline() override = default;

        void Configure(const GraphicsResource::Config* properties) override = 0;

        void Attach(const IGraphicsDriver* driver) final
        {
            m_driver = static_cast<const Driver*>(driver);
        };

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;

    protected:
        const Driver* m_driver;
        vk::raii::Pipeline m_pipeline;
        vk::raii::PipelineLayout m_layout;
    };

    class GraphicsPipeline : public Pipeline
    {
    public:
        struct Config : public Pipeline::Config
        {

        };

    public:
        explicit GraphicsPipeline(const std::string_view& id);
        ~GraphicsPipeline() override;

        void Configure(const GraphicsResource::Config* properties) override;

    protected:
        bool OnLoad() override;
        bool OnUnload() override;
    };
}