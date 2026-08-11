module;

#include <vulkan/vulkan_raii.hpp>

export module Aurion.Vulkan:DescriptorSet;

export namespace Aurion::Vulkan
{
    class DescriptorSet
    {
    public:
        DescriptorSet();
        ~DescriptorSet();

    private:
        vk::raii::DescriptorSet m_set;
    };
}