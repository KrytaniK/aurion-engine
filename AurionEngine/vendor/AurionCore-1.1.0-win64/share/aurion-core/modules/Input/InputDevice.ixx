module;

#include <AurionExport.h>

export module Aurion.Input:Device;

import Aurion.Types;
import :State;

export namespace Aurion
{
	struct AURION_API InputDeviceSpec
	{
		u64 id = 0;
		u64 classification = 0; // Unique classification identifier (think: gamepad, keyboard, etc.)
		const char* alias = nullptr; // Alias for the device, such as "My Keyboard" or "Logitech Gamepad F310"
		const char* p_name = nullptr; // Name of the product as provided by the device
		const char* p_manufacturer = nullptr; // Name of the manufacturer 
		const char* p_version = nullptr; // Product version, as a string 
		const char* p_interface = nullptr; // Name of the interface making the device available (such as HID).
		const char* p_ext = nullptr; // Extension string for interface-specific device capabilities (such as HID information)
	};

	typedef AURION_API u8 InputDeviceMemReqs;

	struct AURION_API InputDeviceCapabilities
	{
		u8 button_count = 0; // Number of buttons on this device
		u8 axis_count = 0; // Number of axes on this device
		u8 pov_count = 0; // Number of POV hats on this device
	};

	class AURION_API InputDevice : public InputState
	{
	public:
		InputDevice(const InputDeviceSpec& spec, const InputDeviceCapabilities& caps, const InputDeviceMemReqs& mem_reqs);
		~InputDevice() = default;

		const InputDeviceSpec& GetSpecification();
		const InputDeviceMemReqs& GetMemoryRequirements();
		const InputDeviceCapabilities& GetCapabilities();

	private:
		InputDeviceSpec m_specs;
		InputDeviceMemReqs m_mem_reqs;
		InputDeviceCapabilities m_caps;
	};
}