#pragma once

#include "CommonTypeDefs.h"
#include "Core/Memory.h"
#include "Core/Platform.h"

namespace Turbo
{
	struct FArenaAllocator
	{
   	static constexpr bool kStaticAllocator = false;

		byte* mAllocation = nullptr;
		byte* mTop = nullptr;
		byte* mTip = nullptr;

	public:

		explicit FArenaAllocator(TurboSize size)
		{
			TRACE_ZONE_SCOPED()

			mAllocation = static_cast<byte*>(Memory::Malloc(size));

			mTip = mAllocation + size;
			mTop = mAllocation;
		}

		~FArenaAllocator() { FPlatform::Free(mAllocation); }

		void* Allocate(TurboSize size)
		{
			TURBO_CHECK(mAllocation != nullptr && mTop != nullptr && mTip != nullptr)
			TURBO_CHECK(size > 0)

			// Align new top
			byte* result = Memory::Align16(mTop);
			byte* newTop = result + size;
			TURBO_CHECK_MSG(newTop <= mTip, "Stack allocator overflow")

			mTop = newTop;

			return result;
		}

		bool Contains(void* ptr, TurboSize size = 0) const
		{
			return ptr >= mAllocation && static_cast<byte*>(ptr) + size <= mTop;
		}

		void Clear() { mTop = mAllocation; }
	};
} // namespace Turbo
