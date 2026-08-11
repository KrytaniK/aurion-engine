module;

#include <string>

export module Aurion.Graphics:IGraphicsDriver;

import Aurion.Resources;
import Aurion.Window;
import Aurion.Types;

import :RenderGraph;

import :Buffer;
import :RenderTarget;
import :Shader;
import :Pipeline;

export namespace Aurion
{
    class IGraphicsDriver
    {
    public:
        virtual ~IGraphicsDriver() = default;

        virtual void DrawFrame(const RenderGraph::CompilationResult& graph) = 0;

        // Creates a blank buffer resource
        virtual ResourceHandle<Buffer> CreateBuffer(const std::string_view& id) = 0;

        // Creates a blank texture, tied to this driver.
        virtual ResourceHandle<Aurion::Texture> CreateTexture(const std::string_view& id) = 0;

        // Creates a blank render target resource
        virtual ResourceHandle<RenderTarget> CreateRenderTarget(const std::string_view& id) = 0;

        // Creates a blank shader resource
        virtual ResourceHandle<Shader> CreateShader(const std::string_view& id) = 0;

        // Creates a blank pipeline resource
        virtual ResourceHandle<Pipeline> CreatePipeline(const std::string_view& id, const Pipeline::Type& type) = 0;
    };
}