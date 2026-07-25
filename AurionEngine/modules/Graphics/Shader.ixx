module;

#include <string>

export module Aurion.Graphics:Shader;

export namespace Aurion
{
    enum class ShaderStage
    {
        Vertex = 0,
        TessellationControl,
        TessellationEval,
        Geometry,
        Fragment,
        Task,
        Mesh
    };

    struct ShaderDescription
    {
        ShaderStage stage = ShaderStage::Vertex;
        std::string path;
    };
}