module;

#include <AurionLog.h>
#include <unordered_map>
#include <memory>
#include <typeindex>

export module Aurion.Services:ServiceLocator;

import :IService;

export namespace Aurion {

    // A global instance to manage localized application services, as a singleton.
    class ServiceLocator
    {
    // Public Methods
    public:
        // Delete Copy Constructor and Assignment operators (No Copying)
        ServiceLocator(const ServiceLocator &) = delete;
        ServiceLocator &operator=(const ServiceLocator &) = delete;

        // Delete Move Constructor and Assignment operators (No Moving)
        ServiceLocator(const ServiceLocator &&) = delete;
        ServiceLocator &operator=(const ServiceLocator &&) = delete;

        // Register an application service in-place
        template<typename T, typename... Args>
        static bool RegisterService(Args&&... args);

        // Register an application service
        // Note: This transfers ownership!
        template<typename T>
        static bool RegisterService(const T&& service);

        // Restart an existing application service
        // Note: This will keep the old service instance!
        template<typename T>
        static bool RestartService();

        // Restart an existing application service with new arguments
        // Note: This destroys the old service instance!
        template<typename T, typename... Args>
        static bool RestartService(Args&&... args);

        // Unregister an existing application service
        template<typename T>
        static bool UnregisterService();

        template<typename T>
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
        ~ServiceLocator() = default;

    // Private Members
    private:
        std::unordered_map<std::type_index, std::unique_ptr<IService>> m_services;
    };
}
