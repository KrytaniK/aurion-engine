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
    void ResourceManager::OnRegister() {}

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

    template <typename T>
    T* ResourceManager::GetResource(const std::string_view& resource_id)
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

        // Check first for any resources of the provided type
        const auto& type_map = m_resource_map.find(std::type_index(typeid(T)));
        if (type_map == m_resource_map.end())
            return nullptr;

        // If any exist, check for the resource by ID
        const u64 hash = std::hash<std::string_view>()(resource_id);
        const auto it = type_map->second.find(hash);
        if (it == type_map->second.end())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }

    template <typename T>
    T* ResourceManager::GetResource(const u64& resource_id)
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

        // Check first for any resources of the provided type
        const auto& type_map = m_resource_map.find(std::type_index(typeid(T)));
        if (type_map == m_resource_map.end())
            return nullptr;

        // If any exist, check for the resource by ID
        const auto it = type_map->second.find(resource_id);
        if (it == type_map->second.end())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }

    template<typename T>
    std::string_view ResourceManager::GetResourceName(const u64& resource_id)
    {
        // Check first for any resources of the provided type
        const auto& type_map = m_resource_map.find(std::type_index(typeid(T)));
        if (type_map == m_resource_map.end())
            return {};

        // If any exist, check for the resource by ID
        const auto it = type_map->second.find(resource_id);
        if (it == type_map->second.end())
            return {};

        return it->second->GetName();
    }

    template <typename T>
    bool ResourceManager::HasResource(const std::string_view& resource_id)
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

        // Check first for any resources of the provided type
        const auto& type_map = m_resource_map.find(std::type_index(typeid(T)));
        if (type_map == m_resource_map.end())
            return false;

        // If any exist, check for the resource by ID
        const auto it = type_map->second.find(std::hash<std::string_view>()(resource_id));
        return it != type_map->second.end();
    }

    template <typename T>
    bool ResourceManager::HasResource(const u64& resource_id)
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

        // Check first for any resources of the provided type
        const auto& type_map = m_resource_map.find(std::type_index(typeid(T)));
        if (type_map == m_resource_map.end())
            return false;

        // If any exist, check for the resource by ID
        const auto it = type_map->second.find(resource_id);
        return it != type_map->second.end();
    }

    template <typename T>
    ResourceHandle<T> ResourceManager::Load(const std::string_view& resource_id)
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

        const auto& type_index = std::type_index(typeid(T));
        const u64 hash = std::hash<std::string_view>()(resource_id);

        // Check first for any resources of the provided type
        const auto& type_map = m_resource_map.find(type_index);
        if (type_map != m_resource_map.end())
        {
            // If the resource exists in the cache, increase ref count and return a handle
            const auto it = type_map->second.find(hash);
            if (it != type_map->second.end())
            {
                ++m_reference_map[type_index][hash].ref_count;
                return ResourceHandle<T>(hash, this);
            }
        }

        // Otherwise, create the resource.
        auto resource = std::make_shared<T>(resource_id);

        // If loading the resource fails, don't cache it; return invalid handle
        if (!resource->Load())
            return ResourceHandle<T>();

        // If loading succeeded, cache the resource
        m_resource_map[type_index][hash] = resource;
        m_reference_map[type_index][hash] = {
            .resource = resource,
            .ref_count = 1,
        };

        // And return a handle
        return ResourceHandle<T>(hash, this);
    }

    template<typename T>
    void ResourceManager::Release(const u64& resource_id)
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

        const auto& type_index = std::type_index(typeid(T));

        const auto refs_by_type = m_reference_map.find(type_index);

        // If no resources of this type exist, simply return
        if (refs_by_type == m_reference_map.end())
        {
            AURION_WARN("[ResourceManager] Failed to release resource with id %d: No resources of type (%s) exist!", resource_id, typeid(T).name());
            return;
        }

        auto it = refs_by_type->second.find(resource_id);

        if (it == refs_by_type->second.end())
        {
            AURION_WARN("[ResourceManager] Failed to release resource with id %d: Resource does not exist!", resource_id);
            return;
        }

        // If no more references remain, unload the resource and remove it from the cache
        if (--it->second.ref_count == 0)
        {
            it->second.resource->Unload();
            refs_by_type->second.erase(it);
            m_resource_map[type_index].erase(resource_id);
        }
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
