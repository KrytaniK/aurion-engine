module;

#include <vulkan/vulkan_raii.hpp>
#include <stdexcept>

module Aurion.Vulkan;

import Aurion.Services;

namespace Aurion::Vulkan
{
    RenderTarget::RenderTarget(const std::string_view& id)
        : Aurion::RenderTarget(id), m_driver(nullptr), m_buffer_index(0)
    {

    }

    void RenderTarget::Configure(const GraphicsResource::Config* properties)
    {
        switch (dynamic_cast<const Config*>(properties)->rtType)
        {
        case RenderTarget::Type::Swapchain:
            {
                m_impl = std::make_unique<RenderTargetImpl_Swapchain>(); break;
            }
        case RenderTarget::Type::Default:
            {
                m_impl = std::make_unique<RenderTargetImpl_Default>(); break;
            }
        }

        m_impl->Configure(m_driver, properties);
    }

    void RenderTarget::Attach(const IGraphicsDriver* driver)
    {
        m_driver = static_cast<const Driver*>(driver);
    }

    void RenderTarget::SwapBuffers()
    {
        m_impl->SwapBuffers(m_buffer_index);
    }

    u32 RenderTarget::GetWidth() const
    {
        return m_impl->GetWidth();
    }

    u32 RenderTarget::GetHeight() const
    {
        return m_impl->GetHeight();

    }

    u32 RenderTarget::GetDepth() const
    {
        return m_impl->GetDepth();
    }

    vk::MemoryRequirements RenderTarget::GetMemoryRequirements() const
    {
        return m_impl->GetMemoryRequirements();
    }

    void RenderTarget::BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset,
        const u32& index) const
    {
        m_impl->BindDeviceMemory(memory, offset, index);
    }

    const vk::raii::ImageView& RenderTarget::GetView() const
    {
        return m_impl->GetView(m_buffer_index);
    }

    vk::Format RenderTarget::GetFormat() const
    {
        return m_impl->GetFormat();
    }

    bool RenderTarget::OnLoad()
    {
        return true; // No-Op: Render targets are blank on creation
    }

    bool RenderTarget::OnUnload()
    {
        return true; // No-Op: vulkan_raii cleans up on its own
    }

    // --------------------------
    // IMPLEMENTATIONS: Default
    // --------------------------

    RenderTargetImpl_Default::RenderTargetImpl_Default()
        : m_driver(nullptr)
    {

    }

    void RenderTargetImpl_Default::Configure(const Driver* driver, const GraphicsResource::Config* properties)
    {
        m_driver = driver;
        m_config = *dynamic_cast<const RenderTarget::Config*>(properties);

        Vulkan::Texture::Config target_cfg;
        target_cfg.image = m_config.image;
        target_cfg.view = m_config.view;
        target_cfg.image_finalLayout = m_config.image_finalLayout;
        target_cfg.width = m_config.width;
        target_cfg.height = m_config.height;
        target_cfg.depth = m_config.depth;

        // Configure target textures
        for (u32 i = 0; i < m_config.frames_in_flight; ++i)
        {
            target_cfg.name = m_config.name + "_" + std::to_string(i);
            m_targets.emplace_back(target_cfg.name);
            m_targets.back().Attach(m_driver);
            m_targets.back().Configure(&target_cfg);
        }

        // Memory requirements will be the same for all in-flight images
        auto mem_reqs = m_targets[0].GetMemoryRequirements();

        // Allocate and bind the targe image to the device memory, and create the image view
        for (u32 i = 0; i < m_config.frames_in_flight; ++i)
        {
            auto memory = m_driver->AllocateDeviceMemory(
                mem_reqs,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
            );

            m_targets[i].BindDeviceMemory(memory, 0);
        }

    }

    u32 RenderTargetImpl_Default::GetWidth() const
    {
        return m_config.width;
    }

    u32 RenderTargetImpl_Default::GetHeight() const
    {
        return m_config.height;
    }

    u32 RenderTargetImpl_Default::GetDepth() const
    {
        return m_config.depth;
    }

    vk::MemoryRequirements RenderTargetImpl_Default::GetMemoryRequirements() const
    {
        return m_targets[0].GetMemoryRequirements();
    }

    void RenderTargetImpl_Default::BindDeviceMemory(
        std::shared_ptr<vk::raii::DeviceMemory>& memory,
        const u32& index,
        const u32& offset
    ) const
    {
        m_targets[index].BindDeviceMemory(memory, offset);
    }

    const vk::raii::ImageView& RenderTargetImpl_Default::GetView(const u32& index) const
    {
        return m_targets[index].GetView();
    }

    vk::Format RenderTargetImpl_Default::GetFormat() const
    {
        return m_config.image.format;
    }

    void RenderTargetImpl_Default::SwapBuffers(u32& index)
    {
        index = (index + 1) % m_config.frames_in_flight;
    }

    // --------------------------
    // IMPLEMENTATIONS: Swapchain
    // --------------------------

    RenderTargetImpl_Swapchain::RenderTargetImpl_Swapchain()
        : m_driver(nullptr), m_win_props(nullptr), m_surface(nullptr), m_swapchain(nullptr)
    {

    }

    void RenderTargetImpl_Swapchain::Configure(const Driver* driver, const GraphicsResource::Config* properties)
    {
        m_driver = driver;
        m_config = *dynamic_cast<const RenderTarget::SwapchainConfig*>(properties);
        m_win_props = &m_config.window->GetProperties();

        // Create the presentation surface
        m_surface = m_driver->CreateWindowSurface(m_config.window);

        // Ensure presentation is supported on said surface
        m_driver->ValidatePresentSupport(m_surface);

        // Create the swapchain
        // TODO: REWORK
        m_swapchain = m_driver->CreateSwapchain(m_surface, m_config, &m_swapchain);

        // Extract the images from the swapchain
        m_images = m_swapchain.getImages();

        for (const auto& image : m_images)
            m_views.emplace_back(m_driver->AllocateImageView(image, m_config.view_config));
    }

    u32 RenderTargetImpl_Swapchain::GetWidth() const
    {
        return m_win_props->width;
    }

    u32 RenderTargetImpl_Swapchain::GetHeight() const
    {
        return m_win_props->height;
    }

    u32 RenderTargetImpl_Swapchain::GetDepth() const
    {
        return 1;
    }

    vk::MemoryRequirements RenderTargetImpl_Swapchain::GetMemoryRequirements() const
    {
        return {};
    }

    void RenderTargetImpl_Swapchain::BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory,
        const u32& offset, const u32& index) const
    {
        // No op for swapchains
    }

    const vk::raii::ImageView& RenderTargetImpl_Swapchain::GetView(const u32& index) const
    {
        return m_views[index];
    }

    vk::Format RenderTargetImpl_Swapchain::GetFormat() const
    {
        return m_config.format;
    }

    void RenderTargetImpl_Swapchain::SwapBuffers(u32& index)
    {
        auto [result, img_idx] = m_swapchain.acquireNextImage(UINT64_MAX, nullptr, nullptr);
        index = img_idx;

        // if (result == vk::Result::eErrorOutOfDateKHR)
        // {
        //     // Recreate Swapchain
        // } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        // {
        //     assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        //     throw std::runtime_error("failed to acquire swap chain image!");
        // }
    }
}
