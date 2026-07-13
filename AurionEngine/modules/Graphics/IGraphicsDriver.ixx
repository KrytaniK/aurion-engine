export module Aurion.Graphics:IGraphicsDriver;

import Aurion.Window;

export namespace Aurion
{
    class IGraphicsDriver
    {
    public:
        virtual ~IGraphicsDriver() = default;

        virtual void BeginFrame() = 0;
        virtual void RecordCommands() = 0;
        virtual void EndFrame() = 0;

        virtual void CreateRenderTarget(const Window* window) = 0;
        //virtual void CreateRenderTarget(/*Params*/) = 0;
    };
}