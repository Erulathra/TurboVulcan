#include "Core/PlatformImplementations/WindowsPlatform.h"

#if PLATFORM_WINDOWS

#include <stdlib.h>
#include "windows.h"
#include "corecrt_malloc.h"

namespace Turbo
{
	bool FWindowsPlatform::IsDebuggerPresent()
	{
	   return ::IsDebuggerPresent();
	}

	void FWindowsPlatform::Sleep(double seconds)
	{
		::Sleep(static_cast<DWORD>(seconds * 1000.f));
	}

	std::optional<std::string> FWindowsPlatform::GetEnviromentalVariable(std::string_view variableName)
	{
	   const std::string nullTerminatedVariableName{variableName.data(), variableName.size()};

		char* buffer = nullptr;
		size_t bufferSize = 0;
		if (_dupenv_s(&buffer, &bufferSize, nullTerminatedVariableName.c_str()))
		{
		   std::string result = std::string(buffer, bufferSize);
			Free(buffer);
         return std::move(result);
		}

		return {};
	}

	void* FWindowsPlatform::Malloc(size_t size)
	{
      return malloc(size);
	}

	void FWindowsPlatform::Free(void *memory)
	{
      free(memory);
	}

	void* FWindowsPlatform::AlignedMalloc(size_t alignment, size_t size)
	{
      return _aligned_malloc(size, alignment);
	}

	void FWindowsPlatform::AlignedFree(void* memory)
	{
      _aligned_free(memory);
	}
} // namespace Turbo

#endif // PLATFORM_WINDOWS
