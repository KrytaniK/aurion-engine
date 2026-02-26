#pragma once

#ifdef AURION_PLATFORM_WINDOWS
	#define AURION_CORE_EXPORT __declspec(dllexport)
	#define AURION_CORE_IMPORT __declspec(dllimport)
#else
	#define AURION_CORE_EXPORT __attribute__((visibility("default")))
	#define AURION_CORE_IMPORT __attribute__((visibility("default")))
#endif

#ifdef AURION_CORE_EXPORTS
	#define AURION_API AURION_CORE_EXPORT
#else
	#define AURION_API AURION_CORE_IMPORT
#endif