module;

#include <AurionExport.h>

export module Aurion.Window:Events;

import Aurion.Types;
import Aurion.Events;
import :Window;
import :Driver;

export namespace Aurion
{

	typedef enum AURION_API WindowEventTypes : u16
	{
		AC_WIN_EVENT_CREATE		= 1 << 0,
		AC_WIN_EVENT_CLOSE		= 1 << 1,
		AC_WIN_EVENT_GET		= 1 << 2
	} WindowEventTypes;

	// Window Category Base Event
	struct AURION_API WindowEvent : EventBase
	{
		WindowEvent() { this->category = AC_EVENT_CATEGORY_WINDOW; };
		WindowEvent(const WindowEventTypes& type) : WindowEvent() { this->type = type; };
		u64 id;
	};

	struct AURION_API WindowCreateEvent : WindowEvent
	{
		WindowCreateEvent() : WindowEvent(AC_WIN_EVENT_CREATE) {};
		WindowProperties properties;
	};

	struct AURION_API WindowCloseEvent : WindowEvent
	{
		WindowCloseEvent() : WindowEvent(AC_WIN_EVENT_CLOSE) {};
	};

	struct AURION_API WindowGetEvent : WindowEvent
	{
		WindowGetEvent() : WindowEvent(AC_WIN_EVENT_GET) {};
		const char* title = nullptr;
		WindowHandle out_handle;
	};
}