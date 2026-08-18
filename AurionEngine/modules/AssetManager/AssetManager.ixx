module;

#include <AurionLog.h>
#include <climits>
#include <string>
#include <unordered_map>
#include <memory>
#include <typeindex>

export module Aurion.Assets:AssetManager;

import :Interface;

import Aurion.Services;

export namespace Aurion
{
    class AssetManager;

    // Asset Handle
    template <typename T>
    class AssetHandle
    {
        using value_type = T;

        static_assert(std::is_base_of_v<IAsset, value_type>, "T must derive from Asset");

    public:
        AssetHandle() : m_asset(nullptr), m_asset_manager(nullptr) {};
        AssetHandle(const u64& id, AssetManager* Asset_manager);

        // Copy Constructors
        AssetHandle(const AssetHandle<T>& other);
        AssetHandle<T>& operator=(const AssetHandle<T>& other);

        // Move Constructors
        AssetHandle(AssetHandle<T>&& other) noexcept;
        AssetHandle<T>& operator=(AssetHandle<T>&& other) noexcept;

        ~AssetHandle();

        [[nodiscard]] std::string_view GetAssetName() const;
        [[nodiscard]] u64 GetId() const;

        T* Get() const;

        template<typename U>
        U* As() const;

        [[nodiscard]] bool IsValid() const;

        // Convenience Operators
        T* operator->() const { return Get(); }
        T& operator*() const { return *Get(); }
        explicit operator bool() const { return IsValid(); }

    private:
        IAsset* m_asset;
        AssetManager* m_asset_manager;
    };

    class AssetManager : public IService
    {
    public:
        struct ReferenceData
        {
            std::shared_ptr<IAsset> Asset = nullptr;
            u64 ref_count = 0;
        };

    public:
        AssetManager() = default;
        ~AssetManager() override = default;

        // Asset Metadata Retrieval
        template <typename T>
        std::string_view GetAssetName(const u64& Asset_id);

        // Asset Checking
        template <typename T>
        bool HasAsset(const std::string_view& Asset_id);
        template <typename T>
        bool HasAsset(const u64& Asset_id);

        // Efficiency Methods

        // Loads a Asset from either the cache (if frequently accessed) or from disk
        template <typename T, typename... Args>
        AssetHandle<T> Load(const std::string_view& Asset_id, Args&&... args);

        // Loads a Asset from the cache/disk, and ties the handle to the base type.
        //  Useful for cases where you wish to preserve common usage through an abstract interface.
        template<typename Base, typename Derived, typename... Args>
        AssetHandle<Base> Load(const std::string_view& Asset_id, Args&&... args);

        // Release a Asset if there are no remaining references
        template <typename T>
        void Release(const u64& Asset_id);

        // Release all Assets from memory
        void UnloadAll();

    private:
        // IService Virtual Overrides
        void OnRegister() override;
        void OnRestart() override;
        void OnUnregister() override;

        // Asset Retrieval
        template <typename T>
        T* GetAsset(const std::string_view& Asset_id);
        template <typename T>
        T* GetAsset(const u64& Asset_id);

        template<typename T>
        void AddReference(const u64& Asset_id);

        // Allows instantiations of AssetHandle to retrieve actual Asset instances and increase ref counts
        template <typename U>
        friend class AssetHandle;

    private:
        // Two-level Asset storage and reference counting. Filter first by type, then by unique ID (Hashed String)
        std::unordered_map<std::type_index, std::unordered_map<u64, std::shared_ptr<IAsset>>> m_asset_map{};
        std::unordered_map<std::type_index, std::unordered_map<u64, ReferenceData>> m_reference_map{};
    };

    template <typename T>
    AssetHandle<T>::AssetHandle(const u64& id, AssetManager* Asset_manager)
        : m_asset_manager(Asset_manager)
    {
        if (!m_asset_manager) return;
        m_asset = m_asset_manager->GetAsset<T>(id);
    }

    template <typename T>
    AssetHandle<T>::AssetHandle(const AssetHandle<T>& other)
        : m_asset(nullptr), m_asset_manager(nullptr)
    {
        // Don't attempt to copy an invalid handle
        if (other.m_asset == nullptr || other.m_asset_manager == nullptr) return;

        // Copy the new reference values
        m_asset = other.m_asset;
        m_asset_manager = other.m_asset_manager;

        // Add a reference on the backend
        m_asset_manager->AddReference<T>(m_asset->GetID());
    }

    template <typename T>
    AssetHandle<T>& AssetHandle<T>::operator=(const AssetHandle<T>& other)
    {
        // Don't attempt to copy an invalid handle, or oneself
        if (other.m_asset_manager == nullptr || this == &other) return *this;

        // If this handle holds a valid Asset, release it
        if (m_asset && m_asset_manager)
            m_asset_manager->Release<T>(m_asset->GetID());

        // Copy the new reference values
        m_asset = other.m_asset;
        m_asset_manager = other.m_asset_manager;

        // Add a reference on the backend
        m_asset_manager->AddReference<T>(m_asset->GetID());

        return *this;
    }

    template <typename T>
    AssetHandle<T>::AssetHandle(AssetHandle<T>&& other) noexcept
        : m_asset(nullptr), m_asset_manager(nullptr)
    {
        // Don't attempt to take over an invalid handle
        if (other.m_asset == nullptr || other.m_asset_manager == nullptr) return;

        // Copy handle Assets
        this->m_asset = other.m_asset;
        this->m_asset_manager = other.m_asset_manager;

        // Invalidate the other handle
        other.m_asset = nullptr;
        other.m_asset_manager = nullptr;
    }

    template <typename T>
    AssetHandle<T>& AssetHandle<T>::operator=(AssetHandle<T>&& other) noexcept
    {
        // Don't attempt to take over an invalid handle, or oneself
        if (other.m_asset_manager == nullptr || this == &other) return *this;

        // If this handle holds a valid Asset, release it
        if (m_asset && m_asset_manager)
            m_asset_manager->Release<T>(m_asset->GetID());

        // Copy handle Assets
        this->m_asset = other.m_asset;
        this->m_asset_manager = other.m_asset_manager;

        // Invalidate the other handle
        other.m_asset = nullptr;
        other.m_asset_manager = nullptr;

        return *this;
    }

    template <typename T>
    AssetHandle<T>::~AssetHandle()
    {
        if (!m_asset_manager || !m_asset) return;

        // Release the held Asset, if available
        m_asset_manager->Release<T>(m_asset->GetID());
    }

    template <typename T>
    std::string_view AssetHandle<T>::GetAssetName() const
    {
        return m_asset->GetAlias();
    }

    template <typename T>
    u64 AssetHandle<T>::GetId() const
    {
        return m_asset->GetID();
    };

    template <typename T>
    T* AssetHandle<T>::Get() const
    {
        return static_cast<T*>(m_asset);
    }

    template <typename T>
    template <typename U>
    U* AssetHandle<T>::As() const
    {
        static_assert(std::is_base_of_v<T, U>, "AssetHandle: Specified type must derive from base type.");

        return static_cast<U*>(m_asset);
    }

    template <typename T>
    bool AssetHandle<T>::IsValid() const
    {
        return m_asset != nullptr;
    }

    template <typename T>
    T* AssetManager::GetAsset(const std::string_view& Asset_id)
    {
        static_assert(std::is_base_of_v<IAsset, T>, "T must derive from Asset");

        // Check first for any Assets of the provided type
        const auto& type_map = m_asset_map.find(std::type_index(typeid(T)));
        if (type_map == m_asset_map.end())
            return nullptr;

        // If any exist, check for the Asset by ID
        const u64 hash = std::hash<std::string_view>()(Asset_id);
        const auto it = type_map->second.find(hash);
        if (it == type_map->second.end())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }

    template <typename T>
    T* AssetManager::GetAsset(const u64& Asset_id)
    {
        static_assert(std::is_base_of_v<IAsset, T>, "T must derive from Asset");

        // Check first for any Assets of the provided type
        const auto& type_map = m_asset_map.find(std::type_index(typeid(T)));
        if (type_map == m_asset_map.end())
            return nullptr;

        // If any exist, check for the Asset by ID
        const auto it = type_map->second.find(Asset_id);
        if (it == type_map->second.end())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }

    template <typename T>
    void AssetManager::AddReference(const u64& Asset_id)
    {
        // Check first for any references to the provided type
        const auto& type_map = m_reference_map.find(std::type_index(typeid(T)));
        if (type_map == m_reference_map.end())
            return;

        // If any exist, filter by ID.
        auto it = type_map->second.find(Asset_id);
        if (it == type_map->second.end())
            return;

        // If any refs to the provided Asset exist, increase ref count
        ++it->second.ref_count;
    }

    template <typename T>
    std::string_view AssetManager::GetAssetName(const u64& Asset_id)
    {
        // Check first for any Assets of the provided type
        const auto& type_map = m_asset_map.find(std::type_index(typeid(T)));
        if (type_map == m_asset_map.end())
            return {};

        // If any exist, check for the Asset by ID
        const auto it = type_map->second.find(Asset_id);
        if (it == type_map->second.end())
            return {};

        return it->second->GetAlias();
    }

    template <typename T>
    bool AssetManager::HasAsset(const std::string_view& Asset_id)
    {
        static_assert(std::is_base_of_v<IAsset, T>, "T must derive from Asset");

        // Check first for any Assets of the provided type
        const auto& type_map = m_asset_map.find(std::type_index(typeid(T)));
        if (type_map == m_asset_map.end())
            return false;

        // If any exist, check for the Asset by ID
        const auto it = type_map->second.find(std::hash<std::string_view>()(Asset_id));
        return it != type_map->second.end();
    }

    template <typename T>
    bool AssetManager::HasAsset(const u64& Asset_id)
    {
        static_assert(std::is_base_of_v<IAsset, T>, "T must derive from Asset");

        // Check first for any Assets of the provided type
        const auto& type_map = m_asset_map.find(std::type_index(typeid(T)));
        if (type_map == m_asset_map.end())
            return false;

        // If any exist, check for the Asset by ID
        const auto it = type_map->second.find(Asset_id);
        return it != type_map->second.end();
    }

    template <typename T, typename... Args>
    AssetHandle<T> AssetManager::Load(const std::string_view& Asset_id, Args&&... args)
    {
        static_assert(std::is_base_of_v<IAsset, T>, "T must derive from Asset");

        const auto& type_index = std::type_index(typeid(T));
        const u64 hash = std::hash<std::string_view>()(Asset_id);

        // Check first for any Assets of the provided type
        const auto& type_map = m_asset_map.find(type_index);
        if (type_map != m_asset_map.end())
        {
            // If the Asset exists in the cache, increase ref count and return a handle
            const auto it = type_map->second.find(hash);
            if (it != type_map->second.end())
            {
                ++m_reference_map[type_index][hash].ref_count;
                return AssetHandle<T>(hash, this);
            }
        }

        // Otherwise, create the Asset.
        auto Asset = std::make_shared<T>(Asset_id, std::forward<Args>(args)...);

        // If loading the Asset fails, don't cache it; return invalid handle
        if (!Asset->IsLoaded())
            return AssetHandle<T>();

        // If loading succeeded, cache the Asset
        m_asset_map[type_index][hash] = Asset;
        m_reference_map[type_index][hash] = {
            .Asset = Asset,
            .ref_count = 1,
        };

        // And return a handle
        return AssetHandle<T>(hash, this);
    }

    template <typename Base, typename Derived, typename... Args>
    AssetHandle<Base> AssetManager::Load(const std::string_view& Asset_id, Args&&... args)
    {
        static_assert(std::is_base_of_v<Base, Derived>, "Derived class must derive from Base class");
        static_assert(std::is_base_of_v<IAsset, Base>, "T must derive from Asset");

        const auto& type_index = std::type_index(typeid(Base));
        const u64 hash = std::hash<std::string_view>()(Asset_id);

        // Check first for any Assets of the provided type
        const auto& type_map = m_asset_map.find(type_index);
        if (type_map != m_asset_map.end())
        {
            // If the Asset exists in the cache, increase ref count and return a handle
            const auto it = type_map->second.find(hash);
            if (it != type_map->second.end())
            {
                ++m_reference_map[type_index][hash].ref_count;
                return AssetHandle<Base>(hash, this);
            }
        }

        // Otherwise, create the Asset.
        auto Asset = std::make_shared<Derived>(Asset_id, std::forward<Args>(args)...);

        // If loading the Asset fails, don't cache it; return invalid handle
        if (!Asset->IsLoaded())
            return AssetHandle<Base>();

        // If loading succeeded, cache the Asset
        m_asset_map[type_index][hash] = Asset;
        m_reference_map[type_index][hash] = {
            .Asset = Asset,
            .ref_count = 1,
        };

        // And return a handle
        return AssetHandle<Base>(hash, this);
    }

    template <typename T>
    void AssetManager::Release(const u64& Asset_id)
    {
        static_assert(std::is_base_of_v<IAsset, T>, "T must derive from Asset");

        const auto& type_index = std::type_index(typeid(T));

        const auto refs_by_type = m_reference_map.find(type_index);

        // If no Assets of this type exist, simply return
        if (refs_by_type == m_reference_map.end())
        {
            AURION_WARN("[AssetManager] Failed to release Asset with id %d: No Assets of type (%s) exist!",
                        Asset_id, typeid(T).name());
            return;
        }

        auto it = refs_by_type->second.find(Asset_id);

        if (it == refs_by_type->second.end())
        {
            AURION_WARN("[AssetManager] Failed to release Asset with id %d: Asset does not exist!",
                        Asset_id);
            return;
        }

        // Deduct a reference to this Asset:
        //  If no more references remain, unload the Asset and remove it from the cache
        if (--it->second.ref_count == 0)
        {
            refs_by_type->second.erase(it);
            m_asset_map[type_index].erase(Asset_id);
        }
    }
}
