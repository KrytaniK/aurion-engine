module;

#include <AurionLog.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <ranges>
#include <typeindex>

module Aurion.Assets;

namespace Aurion
{
    void AssetManager::OnRegister()
    {
    }

    void AssetManager::OnRestart()
    {
        OnUnregister();
        OnRegister();
    }

    void AssetManager::OnUnregister()
    {
        // Clean up and release all existing resources
        UnloadAll();
    }

    void AssetManager::UnloadAll()
    {
        // Clear any and all references FIRST, before destroying resources.
        m_reference_map.clear();
        m_asset_map.clear();
    }
}
