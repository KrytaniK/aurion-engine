module;

#include <AurionExport.h>

export module Aurion.Events:EventBus;

import Aurion.Types;
import :Event;

export namespace Aurion
{
	// Event Category Alias
	typedef AURION_API u16 EventCategoryID;

	// Core Event Handler Callback
	typedef AURION_API void(*EventCallback)(EventBase*, void*);

	constexpr u8 MAX_EVENT_HANDLERS = 16;

	struct AURION_API EventHandler
	{
		EventCategoryID category = AC_EVENT_CATEGORY_NONE; // Guaranteed to be unique
		EventCallback callback = nullptr;
		void* context = nullptr;
	};

	struct AURION_API EventCategoryRegistry
	{
		EventCategoryID category = AC_EVENT_CATEGORY_NONE;
		EventHandler handlers[MAX_EVENT_HANDLERS];
		u8 count = 0;
	};

	class AURION_API EventBus
	{
	public:
		EventBus(const u8& category_count);
		~EventBus();

		void Register(const EventHandler& handler);
		void UnRegister(const EventCategoryID& category, EventCallback callback);

		void Dispatch(EventBase* event);

	private:
		EventCategoryRegistry* m_registry;
		u8 m_registry_count;
		u8 m_max_count;
	};
}