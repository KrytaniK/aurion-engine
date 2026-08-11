module;

#include <vulkan/vulkan_raii.hpp>
#include <string>
#include <vector>

export module Aurion.Vulkan:DescriptorPool;

import Aurion.Resources;
import Aurion.Graphics;
import Aurion.Types;

export namespace Aurion::Vulkan
{
    class Driver;

    class DescriptorPool : public GraphicsResource
    {
    public:
        struct Config : GraphicsResource::Config
        {
            u32 max_sets = 0;
            vk::DescriptorPoolCreateFlags flags{};
            std::vector<vk::DescriptorPoolSize> pool_sizes{};
        };

    public:
        explicit DescriptorPool(const std::string_view& id);
        ~DescriptorPool() override;

        void Configure(const GraphicsResource::Config* properties) override;
        void Attach(const IGraphicsDriver* driver) override;

        std::span<vk::raii::DescriptorSet> AllocateDescriptorSets(const u32& count, const vk::DescriptorSetLayout* layouts);

    protected:
        bool OnLoad() override;
        bool OnUnload() override;

    private:
        Config m_config;
        const Driver* m_driver;
        vk::raii::DescriptorPool m_pool;
        std::vector<vk::raii::DescriptorSet> m_sets;
    };
}