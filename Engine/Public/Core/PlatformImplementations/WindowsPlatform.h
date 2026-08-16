#pragma once

#if PLATFORM_WINDOWS

#include "Core/PlatformImplementations/GenericPlatform.h"

namespace Turbo
{
	struct FWindowsPlatform final : public FGenericPlatform
	{
		static bool IsDebuggerPresent();

		static void Sleep(double seconds);

		static void* Malloc(size_t size);
		static void Free(void* memory);
		static void* AlignedMalloc(size_t alignment, size_t size);
		static void AlignedFree(void* memory);
	};
} // namespace Turbo

#endif // PLATFORM_WINDOWS
