#pragma once

namespace Turbo
{
   /** Always returns nullptr */
	struct FDummyAllocator
	{
      static constexpr bool kStaticAllocator = true;

		static void* Allocate(size_t size) { return nullptr; }
      static void Free(void* data) {}
	};
}
