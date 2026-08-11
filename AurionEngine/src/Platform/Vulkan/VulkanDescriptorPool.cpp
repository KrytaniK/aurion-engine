module;

#include <vulkan/vulkan_raii.hpp>
#include <string>
#include <vector>

module Aurion.Vulkan;

namespace Aurion::Vulkan
{
    DescriptorPool::DescriptorPool(const std::string_view& id)
        : Aurion::GraphicsResource(id), m_config({}), m_driver(nullptr),
            m_pool(nullptr)
    {

    }

    DescriptorPool::~DescriptorPool()
    {

    }

    void DescriptorPool::Configure(const GraphicsResource::Config* properties)
    {
        m_config = *dynamic_cast<const Vulkan::DescriptorPool::Config*>(properties);

        m_sets.reserve(m_config.max_sets);

        m_pool = m_driver->AllocateDescriptorPool(m_config);
    }

    void DescriptorPool::Attach(const IGraphicsDriver* driver)
    {
        m_driver = dynamic_cast<const Vulkan::Driver*>(driver);
    }

    std::span<vk::raii::DescriptorSet> DescriptorPool::AllocateDescriptorSets(const u32& count,
        const vk::DescriptorSetLayout* layouts)
    {
        auto sets = m_driver->AllocateDescriptorSets(m_pool, count, layouts);
        for (auto& set : sets)
            m_sets.push_back(std::move(set));

        return {m_sets.end() - count, m_sets.end()};
    }

    bool DescriptorPool::OnLoad()
    {
        return true;
    }

    bool DescriptorPool::OnUnload()
    {
        return true;
    }
}
