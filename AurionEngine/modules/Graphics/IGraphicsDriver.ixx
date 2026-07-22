module;

#include <string>

export module Aurion.Graphics:IGraphicsDriver;

import Aurion.Resources;
import Aurion.Window;
import Aurion.Types;

import :RenderTarget;

export namespace Aurion
{
    class IGraphicsDriver
    {
    public:
        virtual ~IGraphicsDriver() = default;

        virtual void BeginFrame() = 0;
        virtual void RecordCommands() = 0;
        virtual void EndFrame() = 0;

        // Creates a blank render target
        virtual ResourceHandle<RenderTarget> CreateRenderTarget(const std::string_view& id) = 0;
    };
}