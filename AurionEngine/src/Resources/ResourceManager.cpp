module;

#include <AurionLog.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <ranges>
#include <typeindex>

module Aurion.Resources;

namespace Aurion
{
    void ResourceManager::OnRegister()
    {
    }

    void ResourceManager::OnRestart()
    {
        OnUnregister();
        OnRegister();
    }

    void ResourceManager::OnUnregister()
    {
        // Clean up and release all existing resources
        UnloadAll();
    }

    void ResourceManager::UnloadAll()
    {
        // Clear any and all references FIRST, before destroying resources.
        m_reference_map.clear();

        // Trigger unloading on all resources
        for (auto& resource_map : m_resource_map | std::views::values)
        {
            for (const auto& resource : resource_map | std::views::values)
                resource->Unload();

            resource_map.clear();
        }

        m_resource_map.clear();
    }
}
