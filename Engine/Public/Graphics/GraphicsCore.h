#pragma once

/** Constants */
#include "vulkan/vulkan_core.h"
namespace Turbo
{
	inline constexpr u16 kInvalidSetIndex = I16_MAX;

	inline constexpr u8 kMaxColorAttachments = 8;
	inline constexpr u8 kMaxShaderStages = 5;
	inline constexpr u8 kMaxDescriptorsPerSet = 8;
	inline constexpr u32 kMaxSwapChainImages = 5;
	inline constexpr u32 kMaxFramesInFlight = 2;

	inline constexpr u8 kMaxDescriptorSetLayouts = 2;
	inline constexpr u32 kMaxDescriptorSets = 2;

	inline constexpr u32 kMaxPushConstantSize = 256;

	inline constexpr u32 kVulkanVersion = VK_MAKE_VERSION(1, 3, 0);

	inline constexpr u32 kDefaultTimeout = 1000000000; // 1 second
	inline constexpr u32 kMaxTimeout = U32_MAX;

	using FDeviceAddress = vk::DeviceAddress;
	inline constexpr FDeviceAddress kNullDeviceAddress = 0;

	using FDeviceSize = vk::DeviceSize;
	inline constexpr FDeviceSize kMaxUniformBufferSize = 65536;
}

/** Constants end */

template <>
struct fmt::formatter<vk::Result> : formatter<i32>
{
	format_context::iterator format(vk::Result result, format_context& ctx) const;
};

template <>
struct fmt::formatter<VkResult> : formatter<i32>
{
	format_context::iterator format(VkResult result, format_context& ctx) const;
};

#define CHECK_VULKAN(EXPRESSION) TURBO_CHECK_MSG((EXPRESSION) == VK_SUCCESS, "Vulkan Error: {}", EXPRESSION)
#define CHECK_VULKAN_MSG(EXPRESSION, MESSAGE, ...) TURBO_CHECK_MSG((EXPRESSION) == VK_SUCCESS, "[Vulkan error: {}] " MESSAGE, EXPRESSION __VA_OPT__(,) __VA_ARGS__)

#if TURBO_BUILD_DEVELOPMENT
#define CHECK_VULKAN_HPP(EXPRESSION)																					\
{																														\
	vk::Result _result = (EXPRESSION);																					\
	if (_result == vk::Result::eSuboptimalKHR)																			\
	{																													\
		static bool _subOptimalShown = false;																			\
		if (!_subOptimalShown)																							\
		{																												\
			SPDLOG_WARN("Suboptimal result");																			\
			TURBO_DEBUG_BREAK();																						\
			_subOptimalShown = true;																					\
		}																												\
	}																													\
	else																												\
	{																													\
		TURBO_CHECK_MSG(_result == vk::Result::eSuccess, "Vulkan error: {}", (_result));								\
	}																													\
}
#else // DEBUG
#define CHECK_VULKAN_HPP(EXPRESSION)																					\
{																														\
	vk::Result _result = (EXPRESSION);																					\
	TURBO_CHECK_MSG(_result == vk::Result::eSuccess, "Vulkan error: {}", (_result));									\
}
#endif // else DEBUG

#define CHECK_VULKAN_RESULT(VALUE, EXPRESSION)																				\
{																														\
	vk::Result _resultNew;																								\
	std::tie(_resultNew, VALUE) = EXPRESSION;																			\
	CHECK_VULKAN_HPP(_resultNew);																						\
}

#if !TURBO_BUILD_SHIPPING
#define CHECK_VULKAN_HPP_MSG(EXPRESSION, MESSAGE, ...)																						\
{																																			\
	vk::Result _result = (EXPRESSION);																										\
	if (_result == vk::Result::eSuboptimalKHR)																								\
	{																																		\
		static bool _subOptimalShown = false;																								\
		if (!_subOptimalShown)																												\
		{																																	\
			SPDLOG_WARN("Suboptimal result");																								\
			_subOptimalShown = true;																										\
		}																																	\
	}																																		\
	else																																	\
	{																																		\
		TURBO_CHECK_MSG(_result == vk::Result::eSuccess, "[Vulkan error: {}] " MESSAGE, _result __VA_OPT__(,) __VA_ARGS__);					\
	}																																		\
}
#else
#define CHECK_VULKAN_HPP_MSG(EXPRESSION, MESSAGE, ...) { (void)(EXPRESSION); }
#endif // else TURBO_BUILD_SHIPPING
