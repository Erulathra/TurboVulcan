#pragma once

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;

using iPtr = uintptr_t;

using SizeType = u64;
using ByteType = std::byte;

using fp32 = float;
using fp64 = double;

using ConstString = const char*;

#define I16_MIN std::numeric_limits<i16>::min();
#define I16_MAX std::numeric_limits<i16>::max();
#define U16_MAX std::numeric_limits<u16>::max();

#define I32_MIN std::numeric_limits<i32>::min();
#define I32_MAX std::numeric_limits<i32>::max();
#define U32_MAX std::numeric_limits<u32>::max();

#define I64_MIN std::numeric_limits<i64>::min();
#define I64_MAX std::numeric_limits<i64>::max();
#define U64_MAX std::numeric_limits<u64>::max();

// NOTE(SS): Probably we would get rid of this
#define INDEX_NONE (-1)

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
