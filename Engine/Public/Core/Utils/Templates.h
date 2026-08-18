#pragma once

#include <type_traits>
namespace Turbo
{
	/* I would not gona lie. I was inspired by UE code here. */

	template <typename T>
	constexpr std::remove_reference_t<T>&& MoveTemp(T&& ref)
	{
		using CastType = std::remove_reference_t<T>;

		static_assert(std::is_lvalue_reference_v<T>, "MoveTemp used on rvalue");
		static_assert(!std::is_same_v<CastType&, const CastType&>, "MoveTemp used on rvalue");

		return (CastType&&)ref;
	}

	template <typename T>
	constexpr std::remove_reference_t<T>&& MoveTempIfPosible(T&& ref)
	{
		using CastType = std::remove_reference_t<T>;
		return (CastType&&)ref;
	}

	template <typename T>
	constexpr T&& Forward(std::remove_reference_t<T>& ref)
	{
      return (T&&)ref;
	}

	template <typename T>
	constexpr T&& Forward(std::remove_reference_t<T>&& ref)
	{
      return (T&&)ref;
	}
} // namespace Turbo
