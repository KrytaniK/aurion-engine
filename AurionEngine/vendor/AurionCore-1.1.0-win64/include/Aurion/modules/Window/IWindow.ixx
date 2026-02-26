module;

#include <AurionExport.h>

export module Aurion.Window:Window;

import Aurion.Types;
import Aurion.Input;

export namespace Aurion
{
	typedef enum AURION_API WindowMode : u16
	{
		Windowed					= 1 << 0,
		FullscreenExclusive			= 1 << 1,
		FullscreenBorderless		= 1 << 2
	} WindowMode;

	// Managed Window Properties
	struct AURION_API WindowProperties
	{
		const char* title = nullptr;
		u16 width = 1280;
		u16 height = 720;
		u16 x = 0;
		u16 y = 0;
		WindowMode mode = WindowMode::Windowed;
		bool resizable = true;
		bool minimized = false;
		bool maximized = false;
		bool decorated = true;
		bool focused = true;
		bool fullscreen = false;
	};

	// Core interface declaration
	class AURION_API IWindow
	{
	public:
		virtual ~IWindow() = default;

		// Lifecycle Methods
		virtual void Update(float deltaTime = 0) = 0;

		// Property Access
		virtual void* GetNativeHandle() = 0;
		virtual const WindowProperties& GetProperties() = 0;

		virtual bool IsOpen() = 0;
		virtual bool IsFullscreen(const bool& exclusive = false) = 0;

		// Interactive/Configuration Methods
		virtual void ToggleDecoration() = 0;
		virtual void SetTitle(const char* title) = 0;
		virtual void SetMode(const WindowMode& mode) = 0;
		virtual void Resize(const u16& width, const u16& height) = 0;
		virtual void MoveTo(const u16& x, const u16& y) = 0;
		virtual void Focus() = 0;
		virtual void ToggleMinimize() = 0;
		virtual void ToggleMaximize() = 0;
	};

	// Window utility wrapper class
	class AURION_API Window : public IWindow
	{
	public:
		Window();
		Window(const WindowProperties& properties);
		virtual ~Window() override = default;

		// Update Loop
		virtual void Update(float deltaTime) override = 0;

		virtual void* GetNativeHandle() override final;
		virtual const WindowProperties& GetProperties() override final;

		virtual bool IsOpen() override final;
		virtual bool IsFullscreen(const bool& exclusive) override final;

		virtual void ToggleDecoration() override;
		virtual void SetTitle(const char* title) override;
		virtual void SetMode(const WindowMode& mode) override;
		virtual void Resize(const u16& width, const u16& height) override;
		virtual void MoveTo(const u16& x, const u16& y) override;
		virtual void Focus() override;
		virtual void ToggleMinimize() override;
		virtual void ToggleMaximize() override;

	protected:
		void* m_native_handle;
		WindowProperties m_properties;
	};
}