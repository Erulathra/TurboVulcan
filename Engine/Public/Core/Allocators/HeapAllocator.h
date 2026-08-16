#pragma once

#include "CommonMacros.h"
#include "Core/Platform.h"

namespace Turbo
{
	struct FHeapAllocator
	{
   	static constexpr bool kStaticAllocator = true;

		static void* Allocate(size_t size, size_t alignment = 8)
		{
         TURBO_CHECK(alignment == 1 || alignment == 2 || alignment == 4 || alignment == 8 || alignment == 16)
         return FPlatform::Malloc(size);
		}
		static void Free(void* data)
		{
		   return FPlatform::Free(data);
		}
	};
} // namespace Turbo
