module;

#include <AurionExport.h>

export module Aurion.GLFW:Driver;

import Aurion.Types;
import Aurion.Window;
import Aurion.Events;

import :Window;

export namespace Aurion
{
	class AURION_API GLFWDriver : public IWindowDriver
	{
	public:
		GLFWDriver(const int& client_api = 0, const size_t& max_window_count = 1);
		virtual ~GLFWDriver();

		virtual WindowHandle OpenWindow(const WindowProperties& properties) override;
		virtual bool CloseWindow(const WindowHandle& handle) override;
		virtual bool CloseWindow(const char* title) override;
		virtual bool CloseWindow(const u64& id) override;

		virtual WindowHandle GetWindow(const char* title) override;
		virtual WindowHandle GetWindow(const u64& id) override;

	private:
		size_t m_max_window_count;
		GLFW_Window* m_windows;
	};
}