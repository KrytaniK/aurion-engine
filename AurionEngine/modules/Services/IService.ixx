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
        virtual void OnRegister() = 0;
        virtual void OnRestart() = 0;
        virtual void OnUnregister() = 0;
    };
}
