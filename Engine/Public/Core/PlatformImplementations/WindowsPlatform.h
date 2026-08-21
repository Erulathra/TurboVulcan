#pragma once

#if PLATFORM_WINDOWS

#include "Core/PlatformImplementations/GenericPlatform.h"

namespace Turbo
{
	struct FWindowsPlatform final : public FGenericPlatform
	{
		static bool IsDebuggerPresent();
		static void Sleep(fp64 seconds);
		static std::optional<std::string> GetEnviromentalVariable(std::string_view variableName);

		static void* Malloc(SizeType size);
		static void* Realloc(void* memory, SizeType size);
		static void Free(void* memory);

		static void* AlignedMalloc(size_t alignment, SizeType size);
		static void AlignedFree(void* memory);
	};
} // namespace Turbo

#endif // PLATFORM_WINDOWS
