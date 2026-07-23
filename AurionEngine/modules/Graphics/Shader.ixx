module;

#include <string>

export module Aurion.Graphics:Shader;

export namespace Aurion
{
    enum ShaderStage
    {
        AURION_GPU_SHADER_STAGE_VERTEX = 0,
        AURION_GPU_SHADER_STAGE_FRAGMENT,
    };

    struct ShaderDescription
    {
        ShaderStage stage = AURION_GPU_SHADER_STAGE_VERTEX;
        std::string path;
    };
}