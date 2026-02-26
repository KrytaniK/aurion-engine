module;

#include <AurionExport.h>

export module Aurion.Input:State;

import Aurion.Types;

export namespace Aurion
{
	struct AURION_API InputStateBlock
	{
		u8 offset = 0; // Offset (in bytes) from the start of the parent state block
		u8 total_size = 0; // Total size (in bytes) of this state block
	};

	class AURION_API InputState
	{
	private:
		inline static const InputStateBlock s_invalid_block{0,0};

	public:
		InputState(const u8& total_size = 255u); // Default to max size of 255 bytes
		~InputState();

		InputStateBlock GetStateBlock(const u8& offset, const u8& size);

		void Write(const void* data, const u8& offset, const u8& size);

		void Read(const InputStateBlock& block, void* out_data, const u8& size) const;

		bool IsValidBlock(const InputStateBlock& block) const;

	private:
		u8* m_start_ptr = nullptr; // Pointer to the start of the state memory block
		u8 m_total_size = 0; // Total size (in bytes) of the state memory block
	};
}