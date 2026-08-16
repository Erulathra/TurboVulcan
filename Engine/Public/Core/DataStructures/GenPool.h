#pragma once

#include "CommonMacros.h"
#include "Core/DataStructures/Handle.h"
#include "Core/Memory.h"
#include "spdlog/fmt/bundled/format.h"
#include <cstddef>

namespace Turbo
{
	template <typename Type, FHandle::IndexType size>
		requires(size < FHandle::kMaxIndex)
	struct TGenPool
	{
	public:
		using IndexType = FHandle::IndexType;
		using GenType = FHandle::GenType;

	public:
		Type mData[size];
		bool mUsed[size];
		GenType mGenerations[size];

		IndexType mNextFree[size];
		IndexType mFirstFree = 1;
		IndexType mLastFree = size - 1;

	public:
		TGenPool()
		{
			Memory::MemZero(mData, sizeof(Type) * size);
			Memory::MemZero(mUsed, sizeof(bool) * size);
			Memory::MemZero(mGenerations, sizeof(GenType) * size);

			for (IndexType i = 1; i < (size - 1); ++i)
			{
            mNextFree[i] = i + 1;
			}

			mNextFree[0] = 0;
			mNextFree[size - 1] = 0;
		}

	public:
		static size_t Size() { return size; }

		THandle<Type> Acquire()
		{
			if (IndexType newIndex = FindEmptySlot())
			{
				TURBO_CHECK(mUsed[newIndex] == false)

				mFirstFree = mNextFree[newIndex];
				mNextFree[newIndex] = 0;

				mData[newIndex] = {};
				mUsed[newIndex] = true;
				mGenerations[newIndex] = (mGenerations[newIndex] + 1) % FHandle::kMaxGeneration;

				THandle<Type> NewHandle;
				NewHandle.mIndexAndGen = FHandle::CreateIndex(newIndex, mGenerations[newIndex]);

				// I don't sure, should i do this here.
				mData[newIndex].mHandle = NewHandle;

				return NewHandle;
			}

			TURBO_CHECK_MSG(false, "No more resources left!")
			return THandle<Type>::Nil();
		}

		void Release(THandle<Type> handle)
		{
			if (IndexType slot = DeRef(handle))
			{
				mNextFree[mLastFree] = slot;
				mLastFree = slot;

				mUsed[slot] = false;
			}
		}

		Type* Get(THandle<Type> handle)
		{
			const IndexType index = DeRef(handle);
			return index == 0 ? nullptr : &mData[index];
		}

		const Type* Get(THandle<Type> handle) const
		{
			const IndexType index = DeRef(handle);
			return index == 0 ? nullptr : &mData[index];
		}

		IndexType DeRef(THandle<Type> handle) const
		{
			const IndexType idx = handle.GetIndex();
			const GenType gen = handle.GetGeneration();
			if (idx > 0 && idx < size && mUsed[idx] == true && gen == mGenerations[idx])
			{
				return idx;
			}

			return 0;
		}

		IndexType FindEmptySlot() const
		{
			return mFirstFree;
		}

		template <typename Function>
		void ForEachEntry(Function function)
		{
			for (int ElementId = 0; ElementId < size; ++ElementId)
			{
				if (mUsed[ElementId])
				{
					THandle<Type> newHandle = {};
					newHandle.mIndexAndGen = FHandle::CreateIndex(ElementId, mGenerations[ElementId]);

					function(newHandle);
				}
			}
		}
	};
} // namespace Turbo
