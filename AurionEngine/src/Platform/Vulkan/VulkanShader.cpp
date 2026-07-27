module;

#include <AurionLog.h>
#include <vulkan/vulkan_raii.hpp>

#ifdef AURION_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elifdef AURION_PLATFORM_LINUX
#include <fcntl.h>
#endif

#include <string>
#include <vector>
#include <stdexcept>

module Aurion.Vulkan;

import Aurion.FileSystem;
import Aurion.Types;

namespace Aurion::Vulkan
{
    Shader::Shader(const std::string_view& id)
        : Aurion::Shader(id), m_driver(nullptr), m_file_handle(nullptr),
            m_module(nullptr)
    {}

    Shader::~Shader()
    {

    }

    void Shader::Configure(const GraphicsResource::Config* properties)
    {
        // Save a local copy of the config
        m_config = *dynamic_cast<const Config*>(properties);

        // First, see if a compiled SPIR-V module exists for this shader.
        //  Vulkan will throw an error if this file is not a SPIR-V module.
        // TODO: Implement

        // Generate a file handle to have on hand
        m_file_handle = FSFile(m_config.path.c_str());

        // Open the file to get its contents
        m_file_handle.Open(GetFileAccessParameters());

        if (!m_file_handle.IsOpen())
            throw std::runtime_error("[Vulkan Shader] Failed to open file '" + m_config.path + "'.");

        // Extract file metadata to get size
        FSMetadata metadata = m_file_handle.GetMetadata(true);

        // Clamp the buffer size to a multiple of 4.
        // Vulkan expects this to align with uint32_t
        u64 rem = metadata.size % sizeof(u32);
        u64 clamped_size = rem == 0 ? metadata.size : metadata.size + (sizeof(u32) - rem);

        // Create a temp buffer to hold file contents
        std::vector<char> buffer(clamped_size);

#ifdef AURION_PLATFORM_WINDOWS
        const u64 whence = FILE_BEGIN;
#elifdef AURION_PLATFORM_LINUX
        const u64 whence = SEEK_SET;
#endif

        // Make sure we're at the beginning of the file
        if (m_file_handle.Tell() != 0)
            m_file_handle.Seek(0, whence);

        // Read the file contents into the buffer
        m_file_handle.Read(buffer.data(), metadata.size);

        // Close the file once finished
        m_file_handle.Close();

        // Then, generate the shader module
        m_module = m_driver->CreateShaderModule(buffer);
    }

    void Shader::Attach(const IGraphicsDriver* driver)
    {
        m_driver = static_cast<const Driver*>(driver);
    }

    const std::vector<Shader::EntryPoint>& Shader::GetEntryPoints() const
    {
        return m_config.entry_points;
    }

    const vk::raii::ShaderModule& Shader::GetModule() const
    {
        return m_module;
    }

    bool Shader::OnLoad()
    {
        return true;
    }

    bool Shader::OnUnload()
    {
        return true;
    }

    FSFileOpenParams Shader::GetFileAccessParameters()
    {
        FSFileOpenParams params;

#ifdef AURION_PLATFORM_WINDOWS
        params.dwAccess = GENERIC_READ;
        params.dwShareMode = FILE_SHARE_READ;
        params.lpSecurityAttr = NULL;
        params.dwCreateDisposition = OPEN_EXISTING;
        params.dwFlagsAndAttr = FILE_ATTRIBUTE_NORMAL;
        params.hTemplateFile = NULL;
#elifdef AURION_PLATFORM_LINUX
        params.flags = O_RDONLY;
        params.access = 0;
#endif

        return params;
    }
}
