module;

#include <AurionExport.h>
#include <GLFW/glfw3.h>

export module Aurion.GLFW:Window;

import Aurion.Types;
import Aurion.Window;

export namespace Aurion
{
	class AURION_API GLFW_Window : public Window
	{
	public:
		GLFW_Window();
		GLFW_Window(const WindowProperties& properties);
		virtual ~GLFW_Window() override;

		virtual void Update(float deltaTime) override;
		
		virtual void ToggleDecoration() override;
		virtual void SetTitle(const char* title) override;
		virtual void SetMode(const WindowMode& mode) override;
		virtual void Resize(const u16& width, const u16& height) override;
		virtual void MoveTo(const u16& x, const u16& y) override;
		virtual void Focus() override;
		virtual void ToggleMinimize() override;
		virtual void ToggleMaximize() override;

	private:
		void CreateNativeWindow();

	protected:
		GLFWmonitor* m_native_monitor;
		WindowProperties m_cached_properties;
	};
}