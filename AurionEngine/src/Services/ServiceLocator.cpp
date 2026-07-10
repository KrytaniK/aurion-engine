module;

#include <AurionLog.h>
#include <unordered_map>
#include <memory>
#include <typeindex>

module Aurion.Services;

namespace Aurion
{
    template<typename T, typename... Args>
    bool ServiceLocator::RegisterService(Args&&... args)
    {
        static_assert(std::is_base_of_v<IService, T>, "Service Type must derive from IService");

        auto& type_info = typeid(T);

        // Extract key index from type_info
        auto key = std::type_index(type_info);

        // Only construct the object if the key doesn't exist in the map
        auto& instance = GetInstance();
        auto [iter, success] = instance.m_services.try_emplace(key, std::make_unique<T>(std::forward<Args>(args)...));

        if (!success)
            AURION_ERROR("[ServiceLocator] Failed to register service (%s)", type_info.name());

        // Manually trigger service registration
        iter->second->OnRegister();

        return success;
    }

    template<typename T>
    bool ServiceLocator::RegisterService(const T&& service)
    {
        static_assert(std::is_base_of_v<IService, T>, "Service Type must derive from IService");

        // Extract key index from type_info
        auto key = std::type_index(typeid(T));

        // Only construct the object if the key doesn't exist in the map
        auto& instance = GetInstance();
        auto [iter, success] = instance.m_services.try_emplace(key, std::make_unique<T>(service));

        if (!success)
            AURION_ERROR("[ServiceLocator] Failed to register service (%s)", typeid(T).name());

        // Manually trigger service registration
        iter->second->OnRegister();

        return success;
    }

    template<typename T>
    bool ServiceLocator::RestartService()
    {
        static_assert(std::is_base_of_v<IService, T>, "Service Type must derive from IService");

        auto& type_info = typeid(T);

        // Extract key index from type_info
        const auto key = std::type_index(type_info);

        // Attempt to get the service
        auto& instance = GetInstance();
        const auto iter = instance.m_services.find(key);

        // If not found, don't try to restart
        if (iter == instance.m_services.end())
        {
            AURION_ERROR("[ServiceLocator] Failed to restart service %s: Service does not exist!", type_info.name());
            return false;
        }

        // If found, trigger a restart
        iter->second->OnRestart();

        return true;
    }

    template<typename T, typename... Args>
    bool ServiceLocator::RestartService(Args&&... args)
    {
        static_assert(std::is_base_of_v<IService, T>, "Service Type must derive from IService");

        auto& type_info = typeid(T);

        // Extract key index from type_info
        const auto key = std::type_index(type_info);

        // Attempt to get the service
        auto& instance = GetInstance();
        const auto iter = instance.m_services.find(key);

        // If not found, don't try to restart
        if (iter == instance.m_services.end())
        {
            AURION_ERROR("[ServiceLocator] Failed to restart service %s: Service does not exist!", type_info.name());
            return false;
        }

        // Trigger service unload
        iter->second->OnUnregister();

        // Replace old service with new service (new args)
        iter->second.reset(new T(std::forward<Args>(args)...));

        return true;
    }

    template<typename T>
    bool ServiceLocator::UnregisterService()
    {
        static_assert(std::is_base_of_v<IService, T>, "Service Type must derive from IService");

        auto& type_info = typeid(T);

        // Extract key index from type_info
        const auto key = std::type_index(type_info);

        // Attempt to get the service
        auto& instance = GetInstance();
        const auto iter = instance.m_services.find(key);

        // If not found, don't try to unregister
        if (iter == instance.m_services.end())
        {
            AURION_ERROR("[ServiceLocator] Failed to unregister service %s: Service does not exist!", type_info.name());
            return false;
        }

        // If found, trigger unload and destroy the internal object
        iter->second->OnUnregister();
        iter->second.reset(nullptr);

        // Then, remove the entry from the map
        instance.m_services.erase(iter);
        return true;
    }

    template<typename T>
    T* ServiceLocator::GetService()
    {
        static_assert(std::is_base_of_v<IService, T>, "Service Type must derive from IService");

        auto& type_info = typeid(T);

        // Extract key index from type_info
        const auto key = std::type_index(type_info);

        // Attempt to get the service
        auto& instance = GetInstance();
        const auto iter = instance.m_services.find(key);

        // If not found, return null
        if (iter == instance.m_services.end())
            return nullptr;

        return dynamic_cast<T*>(iter->second.get());
    }
}