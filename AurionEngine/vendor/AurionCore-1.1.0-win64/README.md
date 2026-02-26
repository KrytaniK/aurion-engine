# Aurion Core 1.1

Aurion Core is a C++23 modules-based engine core library providing application lifecycle management, windowing, input handling, an event system, logging, memory allocators, and common types.

## Design Philosophy

Aurion Core is designed as a **foundation for derived implementations**, not a plug-and-play framework. Most modules provide abstract interfaces and base classes that you extend to build your own engine systems. The library prioritizes flexibility over convenience — it gives you the structural scaffolding and contracts, and you provide the concrete behavior.

The one exception is the **GLFW windowing backend** (`Aurion.GLFW`), which ships as a ready-to-use implementation due to the nuanced nature of platform windowing. Even so, the window system is built on top of the same abstract interfaces (`IWindow`, `IWindowDriver`), so you can replace or extend it with a custom implementation if needed.

## Package Contents

```
AurionCore-1.1.0-win64/
  include/
    GLFW/                    # GLFW headers
    Aurion/                  # Aurion headers and modules
      AurionExport.h         # DLL export/import macros
      AurionLog.h            # Logging macros
      modules/               # C++20 module interface units (.ixx)
  lib/
    aurion-core.dll          # Runtime DLL
    aurion-core.lib          # Import library
    glfw3.lib                # GLFW static library
    cmake/
      aurion-core/           # CMake package config (aurion-core)
      glfw3/                 # CMake package config (glfw3)
```

## Modules

| Module               | Description                                                                             |
|----------------------|-----------------------------------------------------------------------------------------|
| `Aurion.Application` | Abstract application base class — derive to define your own lifecycle                   |
| `Aurion.Events`      | Event and EventBus interfaces — derive to define your own event types and dispatch logic |
| `Aurion.GLFW`        | Concrete GLFW windowing backend (ready to use, extensible)                              |
| `Aurion.Input`       | Input interfaces (devices, controls, state) — derive to build your own input system     |
| `Aurion.Log`         | Logging interface with a provided ConsoleLogger — derive for custom loggers             |
| `Aurion.Memory`      | Memory allocator interfaces with Linear, Stack, and Pool implementations                |
| `Aurion.Types`       | Primitive and math types                                                                |
| `Aurion.Window`      | Abstract windowing interfaces (IWindow, IWindowDriver) — see `Aurion.GLFW` for a concrete implementation |

## Requirements

- **C++ Standard:** C++23 (module support required)
- **Compiler:** MSVC 19.34+ (Visual Studio 2022 17.4+) or another compiler with C++23 module support
- **CMake:** 4.2+ (for `find_package` integration)
- **Platform:** Windows (the library uses `__declspec(dllexport/dllimport)`)

## Integration

### CMake

The package ships with CMake config files, so integration is straightforward via `find_package`. Set `CMAKE_PREFIX_PATH` to the package directory and CMake will locate everything automatically.

```cmake
cmake_minimum_required(VERSION 4.2)
project(MyApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "<path-to-AurionCore-1.1.0-win64>")
find_package(glfw3 REQUIRED)
find_package(aurion-core REQUIRED)

add_executable(MyApp main.cpp)

target_link_libraries(MyApp PRIVATE AurionCore::aurion-core)

# Copy runtime DLLs to the output directory
add_custom_command(TARGET MyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:MyApp>
        $<TARGET_FILE_DIR:MyApp>
    COMMAND_EXPAND_LISTS
)
```

Replace `<path-to-AurionCore-1.1.0-win64>` with the path to this package directory (e.g. `C:/libs/AurionCore-1.1.0-win64`).

The `aurion-core` CMake package automatically adds the module `.ixx` files to your target so CMake can compile the required Binary Module Interfaces (BMIs).

#### Visual Studio (CMake via Open Folder or cmake-gui)

1. Open your project folder in Visual Studio (File > Open > Folder) or generate a `.sln` with `cmake -G "Visual Studio 17 2022"`.
2. Ensure your `CMakeLists.txt` includes the configuration above.
3. Build the project. Visual Studio's MSVC compiler will handle the `.ixx` files automatically.
4. The post-build command copies `aurion-core.dll` to your output directory automatically.

#### CLion

1. Open your CMake project in CLion.
2. Ensure your `CMakeLists.txt` includes the configuration above.
3. Go to **Settings > Build, Execution, Deployment > Toolchains** and confirm you are using a Visual Studio (MSVC) toolchain, since the library is built for MSVC on Windows.
4. Build and run.

---

### Premake5 (Visual Studio)

For Premake5, include the `.ixx` module files directly and specify the library and include paths manually.

```lua
workspace "MyApp"
    configurations { "Debug", "Release" }
    architecture "x86_64"

local AURION_DIR = "<path-to-AurionCore-1.1.0-win64>"

project "MyApp"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"

    files {
        "src/**.cpp",
        "src/**.ixx",
        -- Include Aurion module interfaces for BMI compilation
        AURION_DIR .. "/include/Aurion/modules/**.ixx"
    }

    includedirs {
        AURION_DIR .. "/include"
    }

    libdirs {
        AURION_DIR .. "/lib"
    }

    links {
        "aurion-core",
        "glfw3"
    }

    defines { "AURION_PLATFORM_WINDOWS" }

    -- Copy runtime DLL to output directory
    postbuildcommands {
        '{COPYFILE} "' .. AURION_DIR .. '/lib/aurion-core.dll" "%{cfg.targetdir}"'
    }
```

---

## Quick Start

```cpp
// main.cpp
import Aurion.Application;
import Aurion.Log;

class MyApp : public Aurion::Application
{
    void Initialize(int argc, char* argv[]) override
    {
        AURION_INFO("App initialized");
    }

    void Run() override
    {
        while (!m_shouldClose)
        {
            // Main loop
        }
    }

    void Shutdown() override
    {
        AURION_INFO("App shutting down");
    }

    void OnEvent(Aurion::EventBase* event) override
    {
        // Handle events
    }
};

int main(int argc, char* argv[])
{
    MyApp app;
    app.StartAndRun(argc, argv);
    return 0;
}
```

> **Note:** To use the `AURION_INFO(...)` and other logging macros, include the macro header:
> ```cpp
> #include <AurionLog.h>
> ```
