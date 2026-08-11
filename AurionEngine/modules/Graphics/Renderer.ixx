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

        [[nodiscard]] std::shared_ptr<IWindowDriver> GetWindowDriver() const;
        [[nodiscard]] std::shared_ptr<IGraphicsDriver> GetGraphicsDriver() const;

        // Window-Specific Operations
        WindowHandle OpenWindow(const WindowProperties& properties);
        void CloseWindow(const u64& window_id);

        // Render a single frame
        void DrawFrame(const RenderGraph::CompilationResult& graph) const;

        // Creates a blank buffer
        [[nodiscard]] ResourceHandle<Buffer> CreateBuffer(const std::string_view& id) const;

        // Creates a blank render target
        [[nodiscard]] ResourceHandle<RenderTarget> CreateRenderTarget(const std::string_view& id) const;

        // Creates a blank shader
        [[nodiscard]] ResourceHandle<Shader> CreateShader(const std::string_view& id) const;

        // Creates a blank render pipeline
        [[nodiscard]] ResourceHandle<Pipeline> CreatePipeline(const std::string_view& id, const Pipeline::Type& type) const;

    private:
        std::shared_ptr<IWindowDriver> m_window_driver = nullptr; // API-Specific Window Driver (SDL, GLFW, etc.)
        std::shared_ptr<IGraphicsDriver> m_graphics_driver = nullptr; // API-Specific Graphics Driver (Vulkan, OpenGL, etc.)
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
