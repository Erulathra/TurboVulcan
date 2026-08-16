#include "Core/PlatformImplementations/WindowsPlatform.h"

#if PLATFORM_WINDOWS

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
      _aligned_free(pointer);
	}
} // namespace Turbo

#endif // PLATFORM_WINDOWS
