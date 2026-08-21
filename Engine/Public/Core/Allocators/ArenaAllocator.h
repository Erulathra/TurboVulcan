#pragma once

#include "CommonTypeDefs.h"
#include "Core/Memory.h"
#include "Core/Platform.h"

namespace Turbo
{
	struct FArenaAllocator
	{
   	static constexpr bool kStaticAllocator = false;

		ByteType* mAllocation = nullptr;
		ByteType* mTop = nullptr;
		ByteType* mTip = nullptr;

	public:

		explicit FArenaAllocator(SizeType size)
		{
			TRACE_ZONE_SCOPED()

			mAllocation = static_cast<ByteType*>(Memory::Malloc(size));

			mTip = mAllocation + size;
			mTop = mAllocation;
		}

		~FArenaAllocator() { FPlatform::Free(mAllocation); }

		void* Allocate(SizeType size)
		{
			TURBO_CHECK(mAllocation != nullptr && mTop != nullptr && mTip != nullptr)
			TURBO_CHECK(size > 0)

			// Align new top
			ByteType* result = Memory::Align16(mTop);
			ByteType* newTop = result + size;
			TURBO_CHECK_MSG(newTop <= mTip, "Stack allocator overflow")

			mTop = newTop;

			return result;
		}

		bool Contains(void* ptr, SizeType size = 0) const
		{
			return ptr >= mAllocation && static_cast<ByteType*>(ptr) + size <= mTop;
		}

		void Clear() { mTop = mAllocation; }
	};
} // namespace Turbo
