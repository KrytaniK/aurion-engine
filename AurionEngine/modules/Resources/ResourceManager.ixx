module;

#include <AurionLog.h>
#include <climits>
#include <string>
#include <unordered_map>
#include <memory>
#include <typeindex>

export module Aurion.Resources:ResourceManager;

import :Resource;

import Aurion.Services;

export namespace Aurion
{
    class ResourceManager;

    // Resource Handle
    template <typename T>
    class ResourceHandle
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

    public:
        ResourceHandle() : m_resource_id(UINT_MAX), m_resource_manager(nullptr) {};
        ResourceHandle(const u64& id, ResourceManager* resource_manager)
            : m_resource_id(id), m_resource_manager(resource_manager) {};

        ~ResourceHandle();

        [[nodiscard]] std::string_view GetResourceName() const;
        [[nodiscard]] u64 GetId() const;

        T* Get() const;

        [[nodiscard]] bool IsValid() const;

        // Convenience Operators
        T* operator->() const { return Get(); }
        T& operator*() const { return *Get(); }
        explicit operator bool() const { return IsValid(); }

    private:
        u64 m_resource_id{};
        ResourceManager* m_resource_manager;
    };

    class ResourceManager : public IService
    {
    public:
        struct ReferenceData
        {
            std::shared_ptr<Resource> resource = nullptr;
            u64 ref_count = 0;
        };

    public:
        ResourceManager() = default;
        ~ResourceManager() override = default;

        // Resource Metadata Retrieval
        template <typename T>
        std::string_view GetResourceName(const u64& resource_id);

        // Resource Checking
        template <typename T>
        bool HasResource(const std::string_view& resource_id);
        template <typename T>
        bool HasResource(const u64& resource_id);

        // Efficiency Methods

        // Loads a resource from either the cache (if frequently accessed) or from disk
        template <typename T>
        ResourceHandle<T> Load(const std::string_view& resource_id);

        // Loads a resource from the cache/disk, and ties the handle to the base type.
        //  Useful for cases where you wish to preserve common usage through an abstract interface.
        template<typename Base, typename Derived>
        ResourceHandle<Base> Load(const std::string_view& resource_id);

        // Release a resource if there are no remaining references
        template <typename T>
        void Release(const u64& resource_id);

        // Release all resources from memory
        void UnloadAll();

    private:
        // IService Virtual Overrides
        void OnRegister() override;
        void OnRestart() override;
        void OnUnregister() override;

        // Resource Retrieval
        template <typename T>
        T* GetResource(const std::string_view& resource_id);
        template <typename T>
        T* GetResource(const u64& resource_id);

        // Allows instantiations of ResourceHandle to retrieve actual resource instances
        template <typename U>
        friend class ResourceHandle;

    private:
        // Two-level resource storage and reference counting. Filter first by type, then by unique ID (Hashed String)
        std::unordered_map<std::type_index, std::unordered_map<u64, std::shared_ptr<Resource>>> m_resource_map{};
        std::unordered_map<std::type_index, std::unordered_map<u64, ReferenceData>> m_reference_map{};
    };

    template <typename T>
    ResourceHandle<T>::~ResourceHandle()
    {
        if (!m_resource_manager) return;

        // Release the held resource, if available
        m_resource_manager->Release<T>(m_resource_id);
    }

    template <typename T>
    std::string_view ResourceHandle<T>::GetResourceName() const
    {
        return m_resource_manager->GetResourceName<T>(m_resource_id);
    }

    template <typename T>
    u64 ResourceHandle<T>::GetId() const
    {
        return m_resource_id;
    };

    template <typename T>
    T* ResourceHandle<T>::Get() const
    {
        if (!m_resource_manager) return nullptr;
        return m_resource_manager->GetResource<T>(m_resource_id);
    }

    template <typename T>
    bool ResourceHandle<T>::IsValid() const
    {
        return m_resource_manager && m_resource_manager->HasResource<T>(m_resource_id);
    };

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

    template <typename T>
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

    template <typename Base, typename Derived>
    ResourceHandle<Base> ResourceManager::Load(const std::string_view& resource_id)
    {
        static_assert(std::is_base_of_v<Base, Derived>, "Derived class must derive from Base class");
        static_assert(std::is_base_of_v<Resource, Base>, "T must derive from Resource");

        const auto& type_index = std::type_index(typeid(Base));
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
                return ResourceHandle<Base>(hash, this);
            }
        }

        // Otherwise, create the resource.
        auto resource = std::make_shared<Derived>(resource_id);

        // If loading the resource fails, don't cache it; return invalid handle
        if (!resource->Load())
            return ResourceHandle<Base>();

        // If loading succeeded, cache the resource
        m_resource_map[type_index][hash] = resource;
        m_reference_map[type_index][hash] = {
            .resource = resource,
            .ref_count = 1,
        };

        // And return a handle
        return ResourceHandle<Base>(hash, this);
    }

    template <typename T>
    void ResourceManager::Release(const u64& resource_id)
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

        const auto& type_index = std::type_index(typeid(T));

        const auto refs_by_type = m_reference_map.find(type_index);

        // If no resources of this type exist, simply return
        if (refs_by_type == m_reference_map.end())
        {
            AURION_WARN("[ResourceManager] Failed to release resource with id %d: No resources of type (%s) exist!",
                        resource_id, typeid(T).name());
            return;
        }

        auto it = refs_by_type->second.find(resource_id);

        if (it == refs_by_type->second.end())
        {
            AURION_WARN("[ResourceManager] Failed to release resource with id %d: Resource does not exist!",
                        resource_id);
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
}
