module;

#include <vulkan/vulkan_raii.hpp>
#include <memory>

export module Aurion.Vulkan:Buffer;

import Aurion.Graphics;
import Aurion.Types;

export namespace Aurion::Vulkan
{
    class Driver;

    class Buffer : public Aurion::Buffer
    {
    public:
        struct Config : public Aurion::Buffer::Config
        {
            vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eStorageBuffer;
            vk::SharingMode sharing_mode = vk::SharingMode::eExclusive;
            vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostCoherent;
            u32 type_filter = 0;
        };

    public:
        explicit Buffer(const std::string_view& id);
        ~Buffer() override = default;

        void Configure(const GraphicsResource::Config* properties) override ;
        void Attach(const IGraphicsDriver* driver) override;

        void Write(void* data, const u32& offset, const u32& size) override;

        [[nodiscard]] u32 GetSize() const override;

        [[nodiscard]] vk::MemoryRequirements GetMemoryRequirements() const;

        void BindDeviceMemory(const std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset);

        // Convenience Utility for internal buffer access
        const vk::raii::Buffer& operator*() const { return m_buffer; }

    protected:
        bool OnLoad() override;
        bool OnUnload() override;

    private:
        Config m_config;
        const Driver* m_driver{};
        vk::raii::Buffer m_buffer;
        std::shared_ptr<vk::raii::DeviceMemory> m_buffer_memory;
    };
}