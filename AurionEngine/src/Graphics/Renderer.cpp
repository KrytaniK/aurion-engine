module;

#include <typeinfo>

module Aurion.Graphics;

namespace Aurion
{
    Renderer::~Renderer()
    {
        // Explicitly destroy the window/graphics API objects
        m_window_driver.reset(nullptr);
        m_graphics_driver.reset(nullptr);
    }

    WindowHandle Renderer::OpenWindow(const WindowProperties& properties)
    {
        if (!m_window_driver) return {}; // No-op without a window API

        // Create window
        const WindowHandle handle = m_window_driver->OpenWindow(properties);

        // Generate a render target for this window, with a presentation surface
        CreateRenderTarget(handle.window);

        return handle;
    }

    void Renderer::CloseWindow(const u64& window_id)
    {
        if (!m_window_driver) return;
        m_window_driver->CloseWindow(window_id);
    }

    void Renderer::DrawFrame() const
    {
        if (!m_graphics_driver) return;

        m_graphics_driver->BeginFrame();
        m_graphics_driver->RecordCommands();
        m_graphics_driver->EndFrame();
    }

    void Renderer::CreateRenderTarget(const Window* window)
    {
        if (!m_graphics_driver) return;

        //m_graphics_driver->CreateRenderTarget(window);
    }

    void Renderer::SetRenderTarget(const Window* window)
    {
        if (!m_graphics_driver) return;

        //m_graphics_driver->SetRenderTarget(window);
    }
}
