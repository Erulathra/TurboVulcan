#pragma once

namespace Turbo
{
   /** Always returns nullptr */
	struct FDummyAllocator
	{
      static constexpr bool kStaticAllocator = true;

		static void* Allocate(size_t size, size_t alignment = 4) { return nullptr; }
      static void Free(void* data) {}
	};
}
