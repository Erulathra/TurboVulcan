#pragma once

// NOTE(SS): Deprecated aliases
using int8 = std::int8_t;
using uint8 = std::uint8_t;

namespace Turbo {
   // Our graciously reigning Microsoft definied byte in his most roubust and powerfull `Windows.h` library
   using byte = std::byte;
}

using int16 = std::int16_t;
using uint16 = std::uint16_t;

using int32 = std::int32_t;
using uint32 = std::uint32_t;

using int64 = std::int64_t;
using uint64 = std::uint64_t;

using TurboSize = uint64;

// NOTE(SS): New aliases
using i8 = int8_t;
using u8 = uint8_t;
using i16 = uint16_t;
using u16 = uint16_t;
using i32 = uint32_t;
using u32 = uint32_t;
using i64 = uint64_t;
using u64 = uint64_t;

using TSize = uint64;

using fp32 = float;
using fp64 = double;

using ConstString = const char*;

// Deprecated
namespace Turbo
{
	template <typename T>
	using TSharedPtr = std::shared_ptr<T>;

	template <typename T, typename... Args>
	TSharedPtr<T> MakeShared(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	using TWeakPtr = std::weak_ptr<T>;

	template <typename T>
	using TUniquePtr = std::unique_ptr<T>;

	template <typename T, typename... Args>
	TUniquePtr<T> MakeUnique(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
} // namespace Turbo
