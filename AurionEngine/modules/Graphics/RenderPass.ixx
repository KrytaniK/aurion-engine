module;

#include <vector>
#include <string>

export module Aurion.Graphics:RenderPass;

import :GraphicsResource;

export namespace Aurion
{
    // Enum flag determining the classification of a render pass operation.
    enum RenderPassOp
    {
        AURION_GRAPHICS_OP_GRAPHICS =           0x00000000,
        AURION_GRAPHICS_OP_COMPUTE =            0x00000001,
        AURION_GRAPHICS_OP_ASYNC_COMPUTE =      0x00000002,
    };

    // Base POD for describing a render pass. Includes the minimum required
    // members for use within the RenderGraph
    struct RenderPassDescription
    {
        std::string name;
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;

        // Flag used for resolving iternal render pass dependencies
        RenderPassOp op_type = AURION_GRAPHICS_OP_GRAPHICS; // Determines the type of operation this render pass records.
        const u32 channel_index = 0; // The preferred rendering channel.
        const u32 command_buffer_index = 0; // The index of the preferred command buffer
    };
}