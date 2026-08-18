#pragma once

#include "CommonMacros.h"
#include "CommonTypeDefs.h"
#include "Core/Platform.h"

namespace Turbo
{
	struct FHeapAllocator
	{
   	static constexpr bool kStaticAllocator = true;

		static void* Allocate(size_t size)
		{
         return FPlatform::Malloc(size);
		}

		static void* Realloc(void* data, TurboSize newSize)
		{
		   return FPlatform::Realloc(data, newSize);
		}

		static void Free(void* data)
		{
		   return FPlatform::Free(data);
		}

	};
} // namespace Turbo
