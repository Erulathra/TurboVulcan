#pragma once

#include "Core/Platform.h"
#include <bit>
#include <cstddef>
#include <cstring>

namespace Turbo::Memory
{
   constexpr size_t kKibi = 1 << 10;
	constexpr size_t kMebi = 1 << 20;
	constexpr size_t kGibi = 1 << 30;

	template <typename T>
	requires std::is_integral_v<T> || std::is_pointer_v<T>
	constexpr T Align(T value, uintptr_t alignment)
	{
		return reinterpret_cast<T>((reinterpret_cast<uintptr_t>(value) + alignment - 1) & ~(alignment - 1));
	}

	/* memory operations aliases */

	inline void MemSet(void* dst, uint32 bits, size_t count)
	{
	   std::memset(dst, std::bit_cast<int32>(bits), count);
	}

	inline void MemZero(void* dst, size_t numBytes)
	{
      MemSet(dst, 0u, numBytes);
	}

	/* System allocation aliases */

	inline void* AlignedMalloc(size_t alignment, size_t size)
	{
      return FPlatform::AlignedMalloc(alignment, size);
	}

	inline void AlignedFree(void* memory)
	{
	   return FPlatform::AlignedFree(memory);
	}

	/* Allocators helpers */

	template <typename Type, typename AllocatorType>
	Type* Allocate(AllocatorType& allocator, TurboSize num)
	{
		if constexpr (AllocatorType::kStaticAllocator)
		{
   		return (Type*)AllocatorType::Allocate(sizeof(Type) * num, alignof(Type));
		}
		else
		{
         return (Type*)allocator.Allocate(sizeof(Type) * num, alignof(Type));
		}
	}

	template <typename Type, typename AllocatorType>
	Type* Allocate(AllocatorType& allocator)
	{
	   return Allocate<Type>(allocator, 1);
	}

	template <typename Type, typename AllocatorType>
	Type* AllocateZeroed(AllocatorType& allocator, TurboSize num)
	{
	   Type* allocation = Allocate<Type>(allocator, num);
		MemZero(allocation, num * sizeof(Type));
	   return allocation;
	}

	template <typename Type, typename AllocatorType>
	Type* AllocateZeroed(AllocatorType& allocator)
	{
	   return AllocateZeroed<Type>(allocator, 1);
	}

	template <typename Type, typename AllocatorType>
	Type* AllocateDefaulted(AllocatorType& allocator, TurboSize num)
	{
	   Type* allocation = Allocate<Type>(allocator, num);

		for (uint32 i = 0; i < num; i++)
		{
		   allocation[i] = Type{};
		}

	   return allocation;
	}

	template <typename Type, typename AllocatorType>
	Type* AllocateDefaulted(AllocatorType& allocator)
	{
      Type* allocation = Allocate<Type>(allocator);
      *allocation = Type{};

	   return allocation;
	}
}
