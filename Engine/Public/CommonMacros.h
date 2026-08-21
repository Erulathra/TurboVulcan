#pragma once

#if defined(__clang__) == false
#error "Unsupported compiler"
#endif

using namespace std::string_literals;
using namespace std::string_view_literals;

// Debug break
#if DEBUG
#define TURBO_DEBUG_BREAK()                                                                                            \
	{                                                                                                                  \
		if (FPlatform::IsDebuggerPresent())                                                                            \
		{                                                                                                              \
			__builtin_debugtrap();                                                                                     \
		}                                                                                                              \
	}
#else // DEBUG
#define TURBO_DEBUG_BREAK() {};
#endif // else DEBUG

// Assertions
#if WITH_ASSERTIONS
#define TURBO_CHECK(CONDITION)                                                                                         \
	if (!(CONDITION)) [[unlikely]]                                                                                     \
	{                                                                                                                  \
		SPDLOG_ERROR("Assertion `" #CONDITION "` failed.");                                                            \
		spdlog::default_logger_raw()->flush();                                                                         \
		TURBO_DEBUG_BREAK();                                                                                           \
		std::abort();                                                                                                  \
	}
#define TURBO_CHECK_MSG(CONDITION, MESSAGE, ...)                                                                       \
	if (!(CONDITION)) [[unlikely]]                                                                                     \
	{                                                                                                                  \
		SPDLOG_ERROR("Assertion `" #CONDITION "` failed. Message: `" MESSAGE "`" __VA_OPT__(, ) __VA_ARGS__);          \
		spdlog::default_logger_raw()->flush();                                                                         \
		TURBO_DEBUG_BREAK();                                                                                           \
		std::abort();                                                                                                  \
	}
#else // WITH_ASSERTIONS
#define TURBO_CHECK(CONDITION) {}
#define TURBO_CHECK_MSG(CONDITION) {}
#endif // else WITH_ASSERTIONS

#if WITH_SLOW_ASSERTIONS
#define TURBO_CHECK_SLOW(CONDITION) TURBO_CHECK(CONDITION)
#define TURBO_CHECK_MSG_SLOW(CONDITION, MESSAGE, ...) TURBO_CHECK(CONDITION, MESSAGE __VA_OPT(, ) __VA_ARGS__)
#else // WITH_SLOW_ASSERTIONS
#define TURBO_CHECK_SLOW(CONDITION) {}
#define TURBO_CHECK_MSG_SLOW(CONDITION, MESSAGE, ...) {}
#endif // else WITH_SLOW_ASSERTIONS

#define TURBO_STATIC_ASSERT(CONDITION) static_assert(CONDITION, #CONDITION)
#define TURBO_STATIC_ASSERT_MSG(CONDITION, MSG) static_assert(CONDITION, "Static assert '" #CONDITION "' failed. Message: " MSG)

#define UNIMPLEMENTED_BODY()                                                                                           \
	TURBO_DEBUG_BREAK();                                                                                               \
	std::abort()

#define TURBO_UNINPLEMENTED()                                                                                          \
	{                                                                                                                  \
		SPDLOG_ERROR("Unimplemented behaviour.");                                                                      \
		UNIMPLEMENTED_BODY();                                                                                          \
	}

#define TURBO_UNINPLEMENTED_MSG(MESSAGE, ...)                                                                          \
	{                                                                                                                  \
		SPDLOG_ERROR("Unimplemented behaviour. Message: `" MESSAGE "`" __VA_OPT__(, ) __VA_ARGS__);                    \
		UNIMPLEMENTED_BODY();                                                                                          \
	}

#define InvalidDefaultCase                                                                                             \
	default:                                                                                                           \
	{                                                                                                                  \
		TURBO_UNINPLEMENTED();                                                                                           \
	}                                                                                                                  \
	break

#define DEFINE_ENUM_OPERATORS(ENUM_TYPE, INTEGER_TYPE)                                                                 \
	constexpr ENUM_TYPE operator~(ENUM_TYPE rhs) { return static_cast<ENUM_TYPE>(~static_cast<INTEGER_TYPE>(rhs)); }                                                \
	constexpr ENUM_TYPE operator|(ENUM_TYPE lhs, ENUM_TYPE rhs) { return static_cast<ENUM_TYPE>(static_cast<INTEGER_TYPE>(lhs) | static_cast<INTEGER_TYPE>(rhs)); } \
	constexpr ENUM_TYPE operator&(ENUM_TYPE lhs, ENUM_TYPE rhs) { return static_cast<ENUM_TYPE>(static_cast<INTEGER_TYPE>(lhs) & static_cast<INTEGER_TYPE>(rhs)); } \
	constexpr ENUM_TYPE operator^(ENUM_TYPE lhs, ENUM_TYPE rhs) { return static_cast<ENUM_TYPE>(static_cast<INTEGER_TYPE>(lhs) ^ static_cast<INTEGER_TYPE>(rhs)); } \
	constexpr ENUM_TYPE& operator|=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept { return lhs = (lhs | rhs); }              \
	constexpr ENUM_TYPE& operator&=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept { return lhs = (lhs & rhs); }              \
	constexpr ENUM_TYPE& operator^=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept { return lhs = (lhs ^ rhs); }

#define DELETE_COPY(CLASS_NAME)                                                                                        \
	CLASS_NAME(CLASS_NAME const&) = delete;                                                                            \
	CLASS_NAME& operator=(CLASS_NAME const&) = delete;
