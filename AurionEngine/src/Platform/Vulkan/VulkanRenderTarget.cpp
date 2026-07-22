module;

#include <vulkan/vulkan_raii.hpp>

module Aurion.Vulkan;

import Aurion.Services;

namespace Aurion::Vulkan
{
    RenderTarget::RenderTarget(const std::string_view& id)
        : Aurion::RenderTarget(id), m_driver(nullptr), m_current_index(0), m_surface(nullptr), m_swapchain(nullptr)
    {
    }

    void RenderTarget::AssignToDriver(const Driver* driver)
    {
        m_driver = driver;
    }

    void RenderTarget::Configure(const Aurion::RenderTargetProperties* properties)
    {
        // Convert to Vulkan::RenderTargetProperties and copy
        m_config = *static_cast<const RenderTargetProperties*>(properties);
    }

    void RenderTarget::Attach(Window* window)
    {
        m_window = window;

        // IF: Window was provided:
        if (window != nullptr)
        {
            // Create the presentation surface
            m_surface = m_driver->CreateWindowSurface(window);

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

    void RenderTarget::Validate()
    {
        // Validate and recreate swapchain if necessary
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
