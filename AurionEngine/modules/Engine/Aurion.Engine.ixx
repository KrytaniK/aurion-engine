export module Aurion.Engine;

import Aurion.Application;
import Aurion.Events;

export namespace Aurion {
    class Engine : virtual public Application
    {
    public:
        ~Engine() override = default;

    private:
        void Initialize(int argc, char* argv[]) override;
        void OnEvent(EventBase* event) override;
        void Run() override;
        void Shutdown() override;
    };
}