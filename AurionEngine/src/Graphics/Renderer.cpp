module;

#include <AurionLog.h>
#include <memory>
#include <stdexcept>

module Aurion.Graphics;

namespace Aurion
{
    Renderer::~Renderer() = default;

    std::shared_ptr<IWindowDriver> Renderer::GetWindowDriver() const
    {
        return m_window_driver;
    }

    std::shared_ptr<IGraphicsDriver> Renderer::GetGraphicsDriver() const
    {
        return m_graphics_driver;
    }

    WindowHandle Renderer::OpenWindow(const WindowProperties& properties)
    {
        if (!m_window_driver)
            throw std::runtime_error("[Renderer] Failed to open window: No Window API specified!");

        // Create window
        const WindowHandle handle = m_window_driver->OpenWindow(properties);

        return handle;
    }

    void Renderer::CloseWindow(const u64& window_id)
    {
        if (!m_window_driver)
            throw std::runtime_error("[Renderer] Failed to close window: No window API specified!");

        m_window_driver->CloseWindow(window_id);
    }

    void Renderer::DrawFrame(const RenderGraph* graph) const
    {
        if (!m_graphics_driver)
            throw std::runtime_error("[Renderer] Failed to draw frame: No Graphics API specified!");

        m_graphics_driver->DrawFrame(graph);
    }

    ResourceHandle<Buffer> Renderer::CreateBuffer(const std::string_view& id) const
    {
        if (!m_graphics_driver)
            throw std::runtime_error("[Renderer] Failed to create render target: No Graphics API specified!");

        return m_graphics_driver->CreateBuffer(id);
    }

    ResourceHandle<RenderTarget> Renderer::CreateRenderTarget(const std::string_view& id) const
    {
        if (!m_graphics_driver)
            throw std::runtime_error("[Renderer] Failed to create render target: No Graphics API specified!");

        return m_graphics_driver->CreateRenderTarget(id);
    }

    ResourceHandle<Shader> Renderer::CreateShader(const std::string_view& id) const
    {
        if (!m_graphics_driver)
            throw std::runtime_error("[Renderer] Failed to create render target: No Graphics API specified!");

        return m_graphics_driver->CreateShader(id);
    }

    ResourceHandle<Pipeline> Renderer::CreatePipeline(const std::string_view& id, const Pipeline::Type& type) const
    {
        if (!m_graphics_driver)
            throw std::runtime_error("[Renderer] Failed to create render target: No Graphics API specified!");

        return m_graphics_driver->CreatePipeline(id, type);
    }
}
