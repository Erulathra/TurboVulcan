#pragma once

namespace Turbo
{
   /*
    * NOTE(SS): This is brand new more `C` like arena.
    *    [POD and ZII]
    */

   struct Arena
	{
		void* mAllocation = nullptr;
		void* mTop = nullptr;
		void* mTip = nullptr;
		bool bInitialized = false;

		void Init(void* Data, SizeType size)
		{
     		mAllocation = Data;

     		mTip = (ByteType*)mAllocation + size;
     		mTop = (ByteType*)mAllocation;
           bInitialized = true;
		}

		SizeType Size() const
		{
           return (ByteType*)mTip - (ByteType*)mAllocation;
		}

		void* Allocate(SizeType size)
		{
			TURBO_CHECK(mAllocation != nullptr && mTop != nullptr && mTip != nullptr)
			TURBO_CHECK(size > 0)

			// Align new top
			void* result = Memory::Align16(mTop);
			void* newTop = (ByteType*)result + size;
			TURBO_CHECK_MSG(newTop <= mTip, "Arena overflow")

			mTop = newTop;

			return result;
		}

		bool Contains(void* ptr, SizeType size = 0) const
		{
			return ptr >= mAllocation && (ByteType*)(ptr) + size <= mTop;
		}

		void Clear() { mTop = mAllocation; }
	};

};
