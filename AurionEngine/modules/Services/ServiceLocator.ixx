module;

#include <AurionLog.h>
#include <unordered_map>
#include <memory>
#include <ranges>
#include <typeindex>

export module Aurion.Services:ServiceLocator;

import :IService;

export namespace Aurion
{
    // A global instance to manage localized application services, as a singleton.
    class ServiceLocator
    {
        // Public Methods
    public:
        // Delete Copy Constructor and Assignment operators (No Copying)
        ServiceLocator(const ServiceLocator&) = delete;
        ServiceLocator& operator=(const ServiceLocator&) = delete;

        // Delete Move Constructor and Assignment operators (No Moving)
        ServiceLocator(const ServiceLocator&&) = delete;
        ServiceLocator& operator=(const ServiceLocator&&) = delete;

        // Register an application service in-place
        template <typename T, typename... Args>
        static const T* RegisterService(Args&&... args);

        // Register an application service
        // Note: This transfers ownership!
        template <typename T>
        static const T* RegisterService(const T&& service);

        // Restart an existing application service
        // Note: This will keep the old service instance!
        template <typename T>
        static bool RestartService();

        // Restart an existing application service with new arguments
        // Note: This destroys the old service instance!
        template <typename T, typename... Args>
        static bool RestartService(Args&&... args);

        // Unregister an existing application service
        template <typename T>
        static bool UnregisterService();

        template <typename T>
        static T* GetService();

        // Private Methods
    private:
        // Maintain a single access point; Reference for internal use only
        static ServiceLocator& GetInstance()
        {
            static ServiceLocator instance;
            return instance;
        }

        // Private constructor (No explicit instantiation)
        ServiceLocator() = default;

        // Private destructor (Prevent Accidental Deletion)
        ~ServiceLocator();

        // Private Members
    private:
        std::unordered_map<std::type_index, std::unique_ptr<IService>> m_services;
    };

    ServiceLocator::~ServiceLocator()
    {
        // Unregister all services upon destruction.
        // NOTE: This is a fail-safe, in case deserialization isn't handled by the consuming
        //          application. This WILL cause crashing if services are interdependent, since
        //          iteration order may not be guaranteed.
        for (const auto& service : m_services | std::views::values)
            service->OnUnregister();
    }

    template <typename T, typename... Args>
    const T* ServiceLocator::RegisterService(Args&&... args)
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

        return iter->second.get();
    }

    template <typename T>
    const T* ServiceLocator::RegisterService(const T&& service)
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

        return iter->second.get();
    }

    template <typename T>
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

    template <typename T, typename... Args>
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

    template <typename T>
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

    template <typename T>
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
