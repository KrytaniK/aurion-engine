module;

#include <AurionExport.h>
#include <cstdint>

export module Aurion.Types:Primitives;

export namespace Aurion
{
	typedef AURION_API uint8_t u8;
	typedef AURION_API uint16_t u16;
	typedef AURION_API uint32_t u32;
	typedef AURION_API uint64_t u64;

	typedef AURION_API int8_t i8;
	typedef AURION_API int16_t i16;
	typedef AURION_API int32_t i32;
	typedef AURION_API int64_t i64;

	typedef AURION_API float f32;
	typedef AURION_API double f64;
}