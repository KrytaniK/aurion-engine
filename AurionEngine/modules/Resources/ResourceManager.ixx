module;

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
    template <typename T>
    class ResourceHandle;

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

    // Resource Handle
    template <typename T>
    class ResourceHandle
    {
        static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

    public:
        ResourceHandle()
            : m_resource_id(UINT_MAX), m_resource_manager(nullptr)
        {
        };

        ResourceHandle(const u64& id, ResourceManager* resource_manager)
            : m_resource_id(id), m_resource_manager(resource_manager)
        {
        };

        ~ResourceHandle()
        {
            if (!m_resource_manager) return;

            // Release the held resource, if available
            m_resource_manager->Release<T>(m_resource_id);
        }

        std::string_view GetResourceName() const { return m_resource_manager->GetResourceName<T>(m_resource_id); }
        u64 GetId() const { return m_resource_id; };

        T* Get() const
        {
            if (!m_resource_manager) return nullptr;
            return m_resource_manager->GetResource<T>(m_resource_id);
        }

        bool IsValid() const
        {
            return m_resource_manager && m_resource_manager->HasResource<T>(m_resource_id);
        };

        // Convenience Operators
        T* operator->() const
        {
            return Get();
        }

        T& operator*() const
        {
            return *Get();
        }

        explicit operator bool() const
        {
            return IsValid();
        }

    private:
        u64 m_resource_id{};
        ResourceManager* m_resource_manager;
    };
}
