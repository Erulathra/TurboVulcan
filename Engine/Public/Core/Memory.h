#pragma once

#include <bit>
#include <cstddef>
#include <cstring>
namespace Turbo::Memory
{
	template <typename T>
	requires std::is_integral_v<T> || std::is_pointer_v<T>
	constexpr T Align(T value, uintptr_t alignment)
	{
		return reinterpret_cast<T>((reinterpret_cast<uintptr_t>(value) + alignment - 1) & ~(alignment - 1));
	}

	inline void MemSet(void* dst, uint32 bits, size_t count)
	{
	   std::memset(dst, std::bit_cast<int32>(bits), count);
	}

	inline void MemZero(void* dst, size_t count)
	{
      MemSet(dst, 0u, count);
	}
}
