export module Aurion.Engine;

import Aurion.Application;

export namespace Aurion {

    /* Core Systems
        - Virtual File System
        - Asset Management (Caching/Import/Export/etc.)
        - Serialization/Deserialization Mechanisms
        - Rendering (with UI/World separation)
        - Scene Management
        - Physics
        - Audio
        - Input
        - Math Library (likely imported)
        - Better Event/Messaging System
        - Better Memory Management Utilities
        - Profiling Tools
    */

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