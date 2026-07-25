module;

#include <memory>

export module Aurion.Graphics:Renderer;

import Aurion.Window;
import Aurion.Resources;
import Aurion.Types;

import :IGraphicsDriver;
import :RenderGraph;
import :RenderTarget;

export namespace Aurion
{
    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer();

        // Registration method for Window/Graphics API driver classes
        template <typename T, typename... Args>
        void ConfigureDriver(Args&&... args);

        // Pipeline Generation From RenderGraph Output
        void ResolveFrameGraph(const FrameGraph& graph);

        // Window-Specific Operations
        WindowHandle OpenWindow(const WindowProperties& properties);
        void CloseWindow(const u64& window_id);

        // Render a single frame
        void DrawFrame() const;

        // Creates a blank buffer
        ResourceHandle<Buffer> CreateBuffer(const std::string_view& id);

        // Creates a blank render target
        ResourceHandle<RenderTarget> CreateRenderTarget(const std::string_view& id);

    private:
        std::unique_ptr<IWindowDriver> m_window_driver = nullptr; // API-Specific Window Driver (SDL, GLFW, etc.)
        std::unique_ptr<IGraphicsDriver> m_graphics_driver = nullptr; // API-Specific Graphics Driver (Vulkan, OpenGL, etc.)
    };

    template <typename T, typename... Args>
    void Renderer::ConfigureDriver(Args&&... args)
    {
        if constexpr (std::is_base_of_v<IWindowDriver, T>)
            m_window_driver = std::make_unique<T>(std::forward<Args>(args)...);
        else if constexpr (std::is_base_of_v<IGraphicsDriver, T>)
            m_graphics_driver = std::make_unique<T>(std::forward<Args>(args)...);
    }
}
