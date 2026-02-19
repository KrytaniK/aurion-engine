module;

#include <AurionExport.h>

export module Aurion.Input:Control;

import Aurion.Types;
import :Device;
import :State;

export namespace Aurion
{
	class AURION_API InputControl
	{
	public:
		InputControl(InputDevice* device, const InputStateBlock& block);
		~InputControl() = default;

		void Read(void* out_data, const u8& size);

		void Write(void* data, const u8& size);

	protected:
		InputDevice* m_device;
		InputStateBlock m_state;
	};
}