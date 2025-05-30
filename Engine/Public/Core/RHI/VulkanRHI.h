#pragma once

#include "RHICore.h"

#define WITH_VALIDATION_LAYERS DEBUG

#define CHECK_VULKAN_RESULT(RESULT, MESSAGE)									\
if (RESULT != VK_SUCCESS)														\
{																				\
	TURBO_LOG(LOG_RHI, LOG_ERROR, MESSAGE, RESULT);								\
	gEngine->RequestExit(EExitCode::RHICriticalError);							\
	return;																		\
}

template <> struct fmt::formatter<VkResult>: formatter<int32> {
	auto format(VkResult Result, format_context& CTX) const
	  -> format_context::iterator;
};

namespace Turbo
{
	class FSwapChain;
	class FSDLWindow;
	class FVulkanDevice;
	class FVulkanHardwareDevice;

#if WITH_VALIDATION_LAYERS
	// TODO: Move to config
	const static std::vector<const char*> VulkanValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};
#endif // WITH_VALIDATION_LAYERS

	class FVulkanRHI
	{
	private:
		explicit FVulkanRHI();

	public:
		~FVulkanRHI();

	public:
		void Init();
		void InitWindow(FSDLWindow* Window);
		void Destroy();

	private:
		void CreateVulkanInstance();
		void DestroyVulkanInstance();

	private:
		void EnumerateVulkanExtensions();

/** Validation Layers */
#if WITH_VALIDATION_LAYERS

	private:
		bool CheckValidationLayersSupport();

		void SetupValidationLayersCallbacks();
		void DestroyValidationLayersCallbacks();

		static VKAPI_ATTR VkBool32 VKAPI_CALL HandleValidationLayerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT MessageType,
			const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
			void* UserData);
#endif // WITH_VALIDATION_LAYERS
/** Validation Layers End */

	public:
		[[nodiscard]] VkInstance GetVulkanInstance() const { return mVulkanInstance; }

	private:
		void AcquirePhysicalDevice();

	private:
		VkInstance mVulkanInstance = nullptr;
		std::vector<VkExtensionProperties> mExtensionProperties;

#if WITH_VALIDATION_LAYERS
		bool mbValidationLayersEnabled = false;
		VkDebugUtilsMessengerEXT mDebugMessengerHandle;
#endif // WITH_VALIDATION_LAYERS

	private:
		std::unique_ptr<FVulkanHardwareDevice> mHardwareDevice;
		std::unique_ptr<FVulkanDevice> mDevice;
		std::unique_ptr<FSwapChain> mSwapChain;

	public:
		[[nodiscard]] FVulkanHardwareDevice* GetHardwareDevice() const { return mHardwareDevice.get(); }
		[[nodiscard]] FVulkanDevice* GetDevice() const { return mDevice.get(); }
		[[nodiscard]] FSwapChain* GetSwapChainInstance() const { return mSwapChain.get(); }

	public:
		friend class FEngine;
	};
} // namespace Turbo
