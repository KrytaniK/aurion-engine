# C++ Standard 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Source files & Modules
file(GLOB_RECURSE MOD CONFIGURE_DEPENDS "modules/*.ixx")
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE TESTSRC CONFIGURE_DEPENDS "tests/*.cpp")

# Project is a static library
add_library(aurion-engine STATIC ${SRC})

# Ensure consumer module support
target_compile_features(aurion-engine PUBLIC cxx_std_23)

# Project Source
target_sources(aurion-engine
        PUBLIC
            FILE_SET CXX_MODULES FILES ${MOD}
)

target_include_directories(aurion-engine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/modules>
        $<INSTALL_INTERFACE:include/modules>
)

# ----- Project Dependencies
include(FetchContent)

FetchContent_Declare(
        aurion-core
        GIT_REPOSITORY https://github.com/KrytaniK/aurion-core.git
        GIT_TAG main # Always fetch latest
)

FetchContent_MakeAvailable(aurion-core)

# Vulkan
find_package(slang CONFIG HINTS "$ENV{VULKAN_SDK}/lib/cmake")

if (Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    link_libraries(${VULKAN_LIBRARY_DIRS})
    endif()

target_link_libraries(aurion-engine PRIVATE glfw)
target_link_libraries(aurion-engine PRIVATE aurion-core)
target_link_libraries(aurion-engine PRIVATE ${Vulkan_LIBRARIES})

# Setting up build targets for find_package() support
include(GNUInstallDirs)

# Track library for export
install(TARGETS aurion-engine
    EXPORT aurion-engine-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Aurion/modules
)

# Include macro files in the build package
install(DIRECTORY macros/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Aurion)

# Generate the .cmake config file
install(EXPORT aurion-engine-targets
    FILE aurion-engine-config.cmake
        NAMESPACE AurionEngine::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/aurion-engine
)

# Global Defines

# Platform Defines
if (WIN32)
    target_compile_definitions(aurion-engine PRIVATE AURION_PLATFORM_WINDOWS)
elseif (UNIX AND NOT APPLE)
    target_compile_definitions(aurion-engine PRIVATE AURION_PLATFORM_LINUX)
endif()

# Testing

add_executable(aurion-engine-tests ${TESTSRC})

target_link_libraries(aurion-engine-tests PRIVATE glfw)
target_link_libraries(aurion-engine-tests PRIVATE aurion-engine)
target_link_libraries(aurion-engine-tests PRIVATE aurion-core)