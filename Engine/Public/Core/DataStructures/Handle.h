#pragma once

namespace Turbo
{
	struct FHandle
	{
		// Aliases
		using IndexType = u32;
		using GenType = u16;

		// Constants
		static constexpr IndexType kIndexMask = 0xFFFFF;
		static constexpr IndexType kGenerationMask = 0xFFF;

		static constexpr IndexType kMaxIndex = kIndexMask;
		static constexpr IndexType kMaxGeneration = kGenerationMask;

		static constexpr u8 kIndexMaskLength = std::popcount(kIndexMask);
		static constexpr IndexType kNilIndex = 0;

		// Just to make sure that index masks are coherent
		static_assert((kIndexMask | kGenerationMask << kIndexMaskLength) == 0xFFFFFFFF);

		/* Data */
		IndexType mIndexAndGen = kNilIndex;

		/* API */
		static constexpr IndexType CreateIndex(IndexType index, GenType generation)
		{
			return index | (static_cast<IndexType>(generation) << kIndexMaskLength);
		}

		constexpr static FHandle Nil() { return {0}; }

		[[nodiscard]] IndexType GetIndex() const { return mIndexAndGen & kIndexMask; }
		[[nodiscard]] GenType GetGeneration() const { return mIndexAndGen >> kIndexMaskLength; }

		[[nodiscard]] constexpr bool IsValid() const { return mIndexAndGen != kNilIndex; }
		constexpr void Reset() { mIndexAndGen = kNilIndex; }

		explicit constexpr operator bool() const { return IsValid(); }
		constexpr bool operator!() const { return !IsValid(); }

		constexpr bool operator==(const FHandle& rhs) const { return mIndexAndGen == rhs.mIndexAndGen; }
	};

	template <typename ObjectType>
	struct THandle final : public FHandle
	{
		constexpr static THandle<ObjectType> Nil() { return THandle<ObjectType>(FHandle::Nil()); }
	};

	inline bool operator==(const Turbo::FHandle lhs, const Turbo::FHandle rhs)
	{
		return lhs.mIndexAndGen == rhs.mIndexAndGen;
	}

	template <typename T>
	inline bool operator==(const Turbo::THandle<T> lhs, const Turbo::THandle<T> rhs)
	{
		return lhs.mIndexAndGen == rhs.mIndexAndGen;
	}
} // namespace Turbo

namespace std
{
	template <>
	struct hash<Turbo::FHandle>
	{
		size_t operator()(Turbo::FHandle handle) const noexcept { return std::hash<u32>{}(handle.mIndexAndGen); }
	};

	template <typename T>
	struct hash<Turbo::THandle<T>>
	{
		size_t operator()(Turbo::THandle<T> handle) const noexcept { return std::hash<u32>{}(handle.mIndexAndGen); }
	};
} // namespace std
