module;



export module Aurion.Services:IService;

class ServiceLocator;

export namespace Aurion {
    class IService
    {
    public:
        virtual ~IService() = default;

    private:
        friend class ServiceLocator;
        // Called when this service is registered with the ServiceLocator
        virtual void OnRegister() = 0;

        // Called when this service is restarted with the ServiceLocator
        virtual void OnRestart() = 0;

        // Called when this service is unregistered with the ServiceLocator
        virtual void OnUnregister() = 0;
    };
}
