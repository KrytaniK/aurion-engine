module;

#include <vulkan/vulkan_raii.hpp>
#include <stdexcept>

module Aurion.Vulkan;

import Aurion.Services;

namespace Aurion::Vulkan
{
    RenderTarget::RenderTarget(const std::string_view& id)
        : Aurion::RenderTarget(id), m_driver(nullptr), m_current_index(0),
            m_surface(nullptr), m_swapchain(nullptr)
    {
    }

    void RenderTarget::Configure(const GraphicsResource::Config* properties)
    {
        // Save a local configuration copy for swapchain recreation
        m_config = *dynamic_cast<const Config*>(properties);

        // IF: Window was provided:
        if (m_config.window != nullptr)
        {
            // Create the presentation surface
            m_surface = m_driver->CreateWindowSurface(m_config.window);

            // Ensure presentation is supported on said surface
            m_driver->ValidatePresentSupport(m_surface);

            // Create the swapchain
            m_swapchain = m_driver->CreateSwapchain(m_surface, m_config, &m_swapchain);

            // Extract the images from the swapchain
            m_images = m_swapchain.getImages();

            // Create the image views
            m_views = m_driver->CreateImageViews(m_images, m_config);
        }
        // TODO: ELSE:
        // Create <frames_in_flight> images
        // Create <frames_in_flight> image views
    }

    void RenderTarget::Attach(const IGraphicsDriver* driver)
    {
        m_driver = static_cast<const Driver*>(driver);
    }

    void RenderTarget::Validate()
    {
        // Validate and recreate swapchain if necessary
    }

    u32 RenderTarget::GetWidth() const
    {
        return m_config.width;
    }

    u32 RenderTarget::GetHeight() const
    {
        return m_config.height;
    }

    const vk::Image& RenderTarget::GetImage() const
    {
        return m_images[m_current_index];
    }

    const vk::raii::ImageView& RenderTarget::GetView() const
    {
        return m_views[m_current_index];
    }

    vk::Format RenderTarget::GetFormat() const
    {
        return m_config.image.format;
    }

    bool RenderTarget::OnLoad()
    {
        return true; // No-Op: Render targets are blank on creation
    }

    bool RenderTarget::OnUnload()
    {
        return true; // No-Op: vulkan_raii cleans up on its own
    }
}
