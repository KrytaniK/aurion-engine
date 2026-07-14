module;

#include <memory>

export module Aurion.Graphics:Renderer;

import Aurion.Window;
import Aurion.Resources;
import Aurion.Types;

import :IGraphicsDriver;
import :RenderGraph;

export namespace Aurion
{
    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer();

        // Registration method for Window/Graphics API driver classes
        template<typename T, typename... Args>
        void RegisterAPI(Args&&... args);

        // Pipeline Generation From RenderGraph Output
        void BuildPipeline(const RenderPipelineDescription& desc);

        // Window-Specific Operations
        WindowHandle OpenWindow(const WindowProperties& properties);
        void CloseWindow(const u64& window_id);

        // Render a single frame
        void DrawFrame() const;

        // Generates a render target with present support
        // Note: Also sets the current render target
        void CreateRenderTarget(const Window* window);
        // Generates a render target with optional present support
        //void CreateRenderTarget(/*Params*/);

        void SetRenderTarget(const Window* window);

    private:
        std::unique_ptr<IWindowDriver> m_window_driver = nullptr; // API-Specific Window Driver (SDL, GLFW, etc.)
        std::unique_ptr<IGraphicsDriver> m_graphics_driver = nullptr; // API-Specific Graphics Driver (Vulkan, OpenGL, etc.)
    };

    template <typename T, typename ... Args>
    void Renderer::RegisterAPI(Args&&... args)
    {
        if constexpr (std::is_base_of_v<IWindowDriver, T>)
            m_window_driver = std::make_unique<T>(std::forward<Args>(args)...);
        else if constexpr (std::is_base_of_v<IGraphicsDriver, T>)
            m_graphics_driver = std::make_unique<T>(std::forward<Args>(args)...);
    }
}
