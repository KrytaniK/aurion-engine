module;

#include <AurionExport.h>

export module Aurion.Application;

import Aurion.Events;

export namespace Aurion
{
	// Defines an interface for an application, specifying lifecycle methods that must be implemented by derived classes.
	class AURION_API IApplication
	{
	public:
		virtual ~IApplication() = default;

		virtual void StartAndRun(int argc = 0, char* argv[] = nullptr) = 0;

	private:
		virtual void Initialize(int argc = 0, char* argv[] = nullptr) = 0;
		virtual void Run() = 0;
		virtual void Shutdown() = 0;
	};

	// Base application class: Takes over lifecycle management,
	// but does not define implementation.
	class AURION_API Application : public IApplication
	{
	public:
		Application() = default;
		virtual ~Application() override = default;

		virtual void StartAndRun(int argc = 0, char* argv[] = nullptr) override final;

	private:
		virtual void Initialize(int argc = 0, char* argv[] = nullptr) override = 0;
		virtual void Run() override = 0;
		virtual void Shutdown() override = 0;

		virtual void OnEvent(EventBase* event) = 0;

	protected:
		bool m_shouldClose = true;
	};
}