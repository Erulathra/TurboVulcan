#pragma once

#include "CommonMacros.h"
#include "Core/DataStructures/Handle.h"
#include "Core/Memory.h"
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

	public:
		TGenPool()
		{
			Memory::MemZero(mData, sizeof(Type) * size);
			Memory::MemZero(mUsed, sizeof(bool) * size);
			Memory::MemZero(mGenerations, sizeof(GenType) * size);
		}

	public:
		static size_t Size() { return size; }

		THandle<Type> Acquire()
		{
			if (IndexType newIndex = FindEmptySlot())
			{
				TURBO_CHECK(mUsed[newIndex] == false)

				mData[newIndex] = {};
				mUsed[newIndex] = true;
				mGenerations[newIndex] += 1;

				THandle<Type> NewHandle;
				NewHandle.mIndexAndGen = FHandle::CreateIndex(newIndex, mGenerations[newIndex]);

				// I don't sure, should i do this here.
				mData[newIndex].mHandle = NewHandle;

				return NewHandle;
			}

			TURBO_CHECK_MSG(false, "No more resources left!")
			return THandle<Type>::Nil();
		}

		void Release(THandle<Type> handle) { mUsed[DeRef(handle)] = false; }

		Type* Get(THandle<Type> handle) { return &mData[DeRef(handle)]; }

		const Type* Get(THandle<Type> handle) const { return &mData[DeRef(handle)]; }

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
			for (IndexType i = 1; i < size; ++i)
			{
				if (mUsed[i] == false)
				{
					return i;
				}
			}

			return 0;
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
