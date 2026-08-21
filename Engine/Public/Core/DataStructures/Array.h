#pragma once

#include "CommonMacros.h"
#include "CommonTypeDefs.h"
#include "Core/Allocators/HeapAllocator.h"
#include "Core/Memory.h"
#include "Core/Utils/Templates.h"
namespace Turbo
{
	/* Growable array type */
	template <typename T, typename AllocatorType = FHeapAllocator>
	struct TArray
	{
		using Iterator = T*;

		constexpr static fp32 kGrowFactor = 1.5f;

		/* Data */
		T* mData = nullptr;
		SizeType mSize = 0;
		SizeType mCapacity = 0;
		AllocatorType* mAllocator = nullptr;

		/* Constructors */
		explicit TArray(AllocatorType* allocator = nullptr)
			: TArray(4, allocator)
		{
		}

		explicit TArray(SizeType initialCapacity, AllocatorType* allocator = nullptr)
			: mCapacity(initialCapacity)
			, mAllocator(allocator)
		{
			Reserve(initialCapacity);
		}

		/* Methods */
		T* Data() { return mData; }
		const T* Data() const { return mData; }
		SizeType Size() const { return mSize; }
		SizeType ByteSize() const { return mSize * sizeof(T); }
		bool IsEmpty() const { return mSize == 0; }

		template <typename... ArgsType>
		SizeType Emplace(ArgsType&&... args)
		{
			if (mSize + 1 >= mCapacity)
			{
				Grow();
			}

			(void)new (mData + mSize) T(Forward<ArgsType>(args)...);
			++mSize;

			return mSize - 1;
		}

		template <typename... ArgsType>
		T& EmplaceGetRef(ArgsType&&... args)
		{
         SizeType index = Emplace(Forward<ArgsType>(args)...);
         return mData[index];
		}

		SizeType Push(const T& newElement) { return Emplace(newElement); }
		SizeType PushUninitialized(const T& newElement)
		{
			if (mSize + 1 >= mCapacity)
			{
				Grow();
			}

			++mSize;
			return mSize - 1;
		}

		SizeType Pop()
		{
			TURBO_CHECK(IsEmpty() == false)
			mSize--;
		}

		void Clear() { mSize = 0; }

		void Grow() { Reserve(static_cast<SizeType>(mCapacity * kGrowFactor)); }
		void Reserve(SizeType newCapacity)
		{
			if (mData != nullptr)
			{
				mData = Memory::Realloc<T>(mAllocator, mData, newCapacity);
				TURBO_CHECK(mData)
			}
			else
			{
				mData = Memory::Allocate<T>(mAllocator, newCapacity);
			}

			mCapacity = newCapacity;
		}

		/* Operators */
		T& operator[](SizeType index)
		{
			TURBO_CHECK(index > 0 && index < mSize)
			return mData + index;
		}

		const T& operator[](SizeType index) const
		{
			TURBO_CHECK(index > 0 && index < mSize)
			return mData + index;
		}

		/* Iterators */
		Iterator begin() { return mData; }
		Iterator end() { return mData + mSize; }
	};

	/* Non owning view of continues memory */
	template <typename T>
	struct TArrayView
	{
		using Iterator = T*;

		/* Data */
		T* mBegin;
		SizeType mSize;

		/* Constructors */
		TArrayView(T* begin, SizeType size)
			: mBegin(begin)
			, mSize(size)
		{
		}

		template <typename AllocatorType>
		explicit TArrayView(const TArray<T, AllocatorType>& array)
			: mBegin(mBegin)
			, mSize(mSize)
		{
		}

		/* Methods */
		T* Data() { return mBegin; }
		SizeType Size() { return mSize; }

		/* Operators */
		T& operator[](SizeType index)
		{
			TURBO_CHECK(index > 0 && index < mSize)
			return mBegin + index;
		}

		const T& operator[](SizeType index) const
		{
			TURBO_CHECK(index > 0 && index < mSize)
			return mBegin + index;
		}

		/* Iterators */
		Iterator begin() { return mBegin; }
		Iterator end() { return mBegin + mSize; }
	};

} // namespace Turbo
