#pragma once

#include "CommonTypeDefs.h"
#if PLATFORM_LINUX

#include "Core/PlatformImplementations/GenericPlatform.h"

namespace Turbo
{
	struct FLinuxPlatform final : public FGenericPlatform
	{
		static bool IsDebuggerPresent();
		static void Sleep(fp64 seconds);
		static std::optional<std::string> GetEnviromentalVariable(std::string_view variableName);

		static void* Malloc(SizeType size);
		static void* Realloc(void* memory, SizeType newSize);
		static void Free(void* memory);

		static void* AlignedMalloc(SizeType alignment, SizeType size);
		static void AlignedFree(void* memory);
	};
} // namespace Turbo

#endif // PLATFORM_LINUX
