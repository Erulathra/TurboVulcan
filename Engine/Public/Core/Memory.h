#pragma once

#include "CommonMacros.h"
#include "CommonTypeDefs.h"
#include "Core/Platform.h"
#include "glm/ext/scalar_integer.hpp"
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

	inline void* Malloc(size_t size)
	{
      return FPlatform::Malloc(size);
	}

	inline void Free(void* memory)
	{
	   return FPlatform::Free(memory);
	}

	inline void* AlignedMalloc(size_t alignment, size_t size)
	{
      TURBO_CHECK(glm::isPowerOfTwo(alignment))
      return FPlatform::AlignedMalloc(alignment, size);
	}

	inline void AlignedFree(void* memory)
	{
	   return FPlatform::AlignedFree(memory);
	}

	/* Allocators helpers */

	template <typename AllocatorType>
	void* Allocate(AllocatorType* allocator, TurboSize numBytes)
	{
		if constexpr (AllocatorType::kStaticAllocator)
		{
			return AllocatorType::Allocate(numBytes);
		}
		else
		{
			TURBO_CHECK(allocator)
			return allocator->Allocate(numBytes);
		}
	}

	template <typename Type, typename AllocatorType>
	Type* Allocate(AllocatorType* allocator, TurboSize num)
	{
		if constexpr (AllocatorType::kStaticAllocator)
		{
   		return (Type*)AllocatorType::Allocate(sizeof(Type) * num);
		}
		else
		{
         TURBO_CHECK(allocator)
         return (Type*)allocator->Allocate(sizeof(Type) * num);
		}
	}

	template <typename Type, typename AllocatorType>
	Type* Realloc(AllocatorType* allocator, Type* ptr, TurboSize num)
	{
		if constexpr (AllocatorType::kStaticAllocator)
		{
			return (Type*)AllocatorType::Realloc(ptr, sizeof(Type) * num);
		}
		else
		{
			TURBO_CHECK(allocator)
			return (Type*)allocator->Realloc(sizeof(Type) * num);
		}
	}

	template <typename Type, typename AllocatorType>
	Type* Allocate(AllocatorType* allocator)
	{
	   return Allocate<Type>(allocator, 1);
	}

	template <typename Type, typename AllocatorType>
	Type* AllocateZeroed(AllocatorType* allocator, TurboSize num)
	{
	   Type* allocation = Allocate<Type>(allocator, num);
		MemZero(allocation, num * sizeof(Type));
	   return allocation;
	}

	template <typename Type, typename AllocatorType>
	Type* AllocateZeroed(AllocatorType* allocator)
	{
	   return AllocateZeroed<Type>(allocator, 1);
	}

	template <typename Type, typename AllocatorType>
	Type* AllocateDefaulted(AllocatorType* allocator, TurboSize num)
	{
	   Type* allocation = Allocate<Type>(allocator, num);

		for (uint32 i = 0; i < num; i++)
		{
		   allocation[i] = Type{};
		}

	   return allocation;
	}

	template <typename Type, typename AllocatorType>
	Type* AllocateDefaulted(AllocatorType* allocator)
	{
      Type* allocation = Allocate<Type>(allocator);
      *allocation = Type{};

	   return allocation;
	}

	template <typename AllocatorType>
	void* Free(AllocatorType* allocator, void* ptr)
	{
		if constexpr (AllocatorType::kStaticAllocator)
		{
			return AllocatorType::Free(ptr);
		}
		else
		{
			TURBO_CHECK(allocator)
			return allocator->Free(ptr);
		}
	}

}
