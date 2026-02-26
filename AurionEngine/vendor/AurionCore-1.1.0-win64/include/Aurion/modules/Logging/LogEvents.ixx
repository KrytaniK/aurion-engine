module;

#include <AurionExport.h>

export module Aurion.Log:Events;

import Aurion.Types;
import Aurion.Events;
import :Interface;

export namespace Aurion
{
	typedef enum AURION_API LogEventTypes : u16
	{
		AC_LOG_EVENT_MESSAGE = 1 << 0,
		AC_LOG_EVENT_FLUSH = 1 << 1,
		AC_LOG_EVENT_SINK_ADD = 1 << 2,
		AC_LOG_EVENT_SINK_REMOVE = 1 << 3,
		AC_LOG_EVENT_SINK_CLEAR = 1 << 4,
	} LogEventTypes;

	struct AURION_API LogEvent : EventBase
	{
		LogEvent() { this->category = AC_EVENT_CATEGORY_WINDOW; };
		LogEvent(const LogEventTypes& type) : LogEvent() { this->type = type; };

		u16 sink_id = 0; // ID of the logger to use, 0 for none
	};

	struct AURION_API LogMessageEvent : LogEvent
	{
		LogMessageEvent() : LogEvent(AC_LOG_EVENT_MESSAGE) {};
		LogLevel verbosity = AC_LOG_LEVEL_TRACE;
		const char* message = nullptr;
	};

	struct AURION_API LogFlushEvent : LogEvent
	{
		LogFlushEvent() : LogEvent(AC_LOG_EVENT_FLUSH) {};
	};

	struct AURION_API LogSinkAddEvent : LogEvent
	{
		LogSinkAddEvent() : LogEvent(AC_LOG_EVENT_SINK_ADD) {};
		
	};

	struct AURION_API LogSinkRemoveEvent : LogEvent
	{
		LogSinkRemoveEvent() : LogEvent(AC_LOG_EVENT_SINK_REMOVE) {};
	};

	struct AURION_API LogSinkClearEvent : LogEvent
	{
		LogSinkClearEvent() : LogEvent(AC_LOG_EVENT_SINK_CLEAR) {};
	};
}