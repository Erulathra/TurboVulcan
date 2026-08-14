#pragma once

namespace Turbo
{
	template<typename Type, FHandle::IndexType size>
		requires (size < FHandle::kMaxIndex)
	class TGenPool
	{
	private:
		std::array<Type, size> mHotData;
		std::array<FHandle::GenerationType, size> mGenerations;
		std::array<FHandle::IndexType, size> mFreeIndices;

		FHandle::IndexType mFreeIndicesHead = 0;
		FHandle::IndexType mUsedIndices = 0;

	public:
		TGenPool() { Clear(); }

	public:
		[[nodiscard]] static constexpr FHandle::IndexType GetCapacity() { return size; }
		[[nodiscard]] FHandle::IndexType GetNumAcquiredResources() const { return mUsedIndices; }

		void Clear()
		{
			mFreeIndicesHead = 0;
			mUsedIndices = 0;

			for (FHandle::IndexType i = 0; i < size; ++i)
			{
				mFreeIndices[i] = i;
			}

			for (FHandle::IndexType i = 0; i < size; ++i)
			{
				mGenerations[i] = 0;
			}
		}

		THandle<Type> Acquire()
		{
			TURBO_CHECK_MSG(mFreeIndicesHead < size, "No more resources left!")

			FHandle::IndexType newIndex = mFreeIndices[mFreeIndicesHead];

			THandle<Type> NewHandle {};
			NewHandle.mIndexAndGen = FHandle::CreateIndex(newIndex, mGenerations[newIndex]);
#if 0
			TURBO_CHECK_MSG(NewHandle.GetGeneration() < FHandle::kMaxGeneration, "No more resource generations left!")
#endif

			++mFreeIndicesHead;
			++mUsedIndices;

			return NewHandle;
		}

		void Release(THandle<Type> handle)
		{
			TURBO_CHECK(mUsedIndices > 0)

			--mFreeIndicesHead;
			--mUsedIndices;

			mFreeIndices[mFreeIndicesHead] = handle.GetIndex();

			++mGenerations[handle.GetIndex()];
		}

		Type* Access(THandle<Type> handle)
		{
			if (handle.IsValid() && handle.GetIndex() < mHotData.size())
			{
				if (handle.GetGeneration() == mGenerations[handle.GetIndex()])
				{
					return &mHotData[handle.GetIndex()];
				}
			}

			return nullptr;
		}

		const Type* Access(THandle<Type> handle) const
		{
			if (handle.IsValid() && handle.GetIndex() < mHotData.size())
			{
				if (handle.GetGeneration() == mGenerations[handle.GetIndex()])
				{
					return &mHotData[handle.GetIndex()];
				}
			}

			return nullptr;
		}

		template<typename Function>
		void ForEachEntry(Function function)
		{
			for (int ElementId = 0; ElementId < mFreeIndicesHead; ++ElementId)
			{
				FHandle::IndexType newIndex = mFreeIndices[ElementId];

				THandle<Type> newHandle = {};
				newHandle.mIndexAndGen = FHandle::CreateIndex(newIndex, mGenerations[newIndex]);
				function(newHandle);
			}
		}

	};
} // turbo
