module;

#include <stdexcept>

module Aurion.Graphics;

namespace Aurion
{
    Renderer::~Renderer()
    {
        // Explicitly destroy the window/graphics API objects
        m_window_driver.reset(nullptr);
        m_graphics_driver.reset(nullptr);
    }

    void Renderer::BuildPipeline(const RenderPipelineDescription& desc)
    {
        if (!m_graphics_driver)
            throw std::runtime_error("[Renderer] Failed to build graphics pipeline: No Graphics API specified!");

        // Generate all resources on the backend
        for (const auto& resource_desc : desc.resource_descriptions)
        {
        }
        // m_graphics_driver->CreateResource(resource_desc);
    }

    WindowHandle Renderer::OpenWindow(const WindowProperties& properties)
    {
        if (!m_window_driver)
            throw std::runtime_error("[Renderer] Failed to open window: No Window API specified!");

        // Create window
        const WindowHandle handle = m_window_driver->OpenWindow(properties);

        // Generate a render target for this window, with a presentation surface
        CreateRenderTarget(handle.window);

        return handle;
    }

    void Renderer::CloseWindow(const u64& window_id)
    {
        if (!m_window_driver)
            throw std::runtime_error("[Renderer] Failed to close window: No window API specified!");

        m_window_driver->CloseWindow(window_id);
    }

    void Renderer::DrawFrame() const
    {
        if (!m_graphics_driver)
            throw std::runtime_error("[Renderer] Failed to draw frame: No Graphics API specified!");

        m_graphics_driver->BeginFrame();
        m_graphics_driver->RecordCommands();
        m_graphics_driver->EndFrame();
    }

    void Renderer::CreateRenderTarget(const Window* window)
    {
        if (!m_graphics_driver)
            throw std::runtime_error("[Renderer] Failed to create render target: No Graphics API specified!");

        //m_graphics_driver->CreateRenderTarget(window);
    }

    void Renderer::SetRenderTarget(const Window* window)
    {
        if (!m_graphics_driver)
            throw std::runtime_error("[Renderer] Failed to set render target: No Graphics API specified!");

        //m_graphics_driver->SetRenderTarget(window);
    }
}
