module;

#include <string>
#include <vector>

export module Aurion.Graphics:Shader;

import Aurion.Types;

import Aurion.FileSystem;

import :GraphicsResource;

export namespace Aurion
{
    class Shader : public GraphicsResource
    {
    public:
        // Supported shader languages
        enum Language
        {
            HLSL = 0,
            GLSL,
            SPIRV
        };

        struct Macro
        {
            std::string key;
            std::string value;
        };

        // Supported pipeline shader stages/types
        enum Stage
        {
            Vertex = 0,
            TessellationControl,
            TessellationEval,
            Geometry,
            Fragment,
            Task,
            Mesh
        };

        struct EntryPoint
        {
            Stage stage = Vertex;
            std::string name = "Main";
        };

        struct Config : GraphicsResource::Config
        {
            explicit Config() : GraphicsResource::Config(GraphicsResource::Shader) {};

            Language lang = HLSL;
            std::vector<Macro> defines{};
            std::vector<EntryPoint> entry_points{};
            std::string path{};
        };

    public:
        explicit Shader(const std::string_view& id) : GraphicsResource(id) {};
        ~Shader() override = default;

        void Configure(const GraphicsResource::Config* properties) override = 0;
        void Attach(const IGraphicsDriver* driver) override = 0;

    protected:
        bool OnLoad() override = 0;
        bool OnUnload() override = 0;

    private:
        // Get file open/access parameters, based on API implementation
        virtual FSFileOpenParams GetFileAccessParameters() = 0;
    };
}