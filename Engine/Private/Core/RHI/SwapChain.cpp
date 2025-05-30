#include "Core/RHI/SwapChain.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/Math/Math.h"
#include "Core/Math/Vector2D.h"
#include "Core/RHI/VulkanHardwareDevice.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;

void FSwapChain::Init(const FVulkanDevice* InDevice)
{
	TURBO_CHECK(IsValid(InDevice));

	const FVulkanHardwareDevice* HWDevice = gEngine->GetRHI()->GetHardwareDevice();

	TURBO_LOG(LOG_RHI, LOG_INFO, "Creating Swap chain");

	const SwapChainDeviceSupportDetails SupportDetails = HWDevice->QuerySwapChainSupport();

	uint32 ImageCount = SupportDetails.Capabilities.minImageCount + 1;
	if (SupportDetails.Capabilities.maxImageCount > 0)
	{
		ImageCount = FMath::Min(ImageCount, SupportDetails.Capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR CreateInfo{};
	CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	CreateInfo.surface = gEngine->GetWindow()->GetVulkanSurface();

	CreateInfo.minImageCount = ImageCount;
	VkSurfaceFormatKHR PixelFormat = SelectBestSurfacePixelFormat(SupportDetails.Formats);
	CreateInfo.imageFormat = PixelFormat.format;
	CreateInfo.imageColorSpace = PixelFormat.colorSpace;
	CreateInfo.imageExtent = CalculateSwapChainExtent(SupportDetails.Capabilities);
	CreateInfo.imageArrayLayers = 1;
	CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	const FQueueFamilyIndices& QueueIndices = InDevice->GetQueueIndices();
	const std::set<uint32> UniqueQueueIndices = QueueIndices.GetUniqueQueueIndices();
	const std::vector<uint32> QueueIndicesVector(UniqueQueueIndices.begin(), UniqueQueueIndices.end());
	if (QueueIndices.GraphicsFamily == QueueIndices.PresentFamily)
	{
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		CreateInfo.queueFamilyIndexCount = QueueIndices.Num();
		CreateInfo.pQueueFamilyIndices = QueueIndicesVector.data();
	}

	CreateInfo.preTransform = SupportDetails.Capabilities.currentTransform;
	CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	CreateInfo.presentMode = SelectBestPresentMode(SupportDetails.PresentModes);
	CreateInfo.clipped = VK_TRUE;

	CreateInfo.oldSwapchain = nullptr;

	const VkResult SwapChaindCreationResult = vkCreateSwapchainKHR(InDevice->GetVulkanDevice(), &CreateInfo, nullptr, &mVulkanSwapChain);
	CHECK_VULKAN_RESULT(SwapChaindCreationResult, "Error: {} during creating swap chain.");

	mImageFormat = CreateInfo.imageFormat;
	mImageSize = CreateInfo.imageExtent;

	uint32 SwapChainImagesNum;
	vkGetSwapchainImagesKHR(InDevice->GetVulkanDevice(), mVulkanSwapChain, &SwapChainImagesNum, nullptr);
	mImages.resize(SwapChainImagesNum);
	vkGetSwapchainImagesKHR(InDevice->GetVulkanDevice(), mVulkanSwapChain, &ImageCount, mImages.data());

	InitializeImageViews(InDevice);
}


void FSwapChain::Destroy()
{
	TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying Swap chain");

	const FVulkanDevice* Device = gEngine->GetRHI()->GetDevice();
	TURBO_CHECK(Device);

	VkDevice VulkanDevice = Device->GetVulkanDevice();
	vkDestroySwapchainKHR(VulkanDevice, mVulkanSwapChain, nullptr);

	for (VkImageView& ImageView : mImageViews)
	{
		vkDestroyImageView(VulkanDevice, ImageView, nullptr);
	}
	mImageViews.clear();
}

VkSurfaceFormatKHR FSwapChain::SelectBestSurfacePixelFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats) const
{
	TURBO_CHECK(!AvailableFormats.empty());

	for (const VkSurfaceFormatKHR& Format : AvailableFormats)
	{
		if (Format.format == VK_FORMAT_B8G8R8A8_SRGB
			&& Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return Format;
		}
	}

	return AvailableFormats.front();
}

VkPresentModeKHR FSwapChain::SelectBestPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes) const
{
	TURBO_CHECK(!AvailableModes.empty());

	if (std::ranges::contains(AvailableModes, VK_PRESENT_MODE_IMMEDIATE_KHR))
	{
		return VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D FSwapChain::CalculateSwapChainExtent(const VkSurfaceCapabilitiesKHR& Capabilities) const
{
	FUIntVector2 FramebufferSize = gEngine->GetWindow()->GetFrameBufferSize();
	FramebufferSize.x = FMath::Clamp(FramebufferSize.x, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
	FramebufferSize.y = FMath::Clamp(FramebufferSize.y, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

	return  {FramebufferSize.x, FramebufferSize.y};
}

void FSwapChain::InitializeImageViews(const FVulkanDevice* Device)
{
	TURBO_LOG(LOG_RHI, LOG_DISPLAY, "Creating swap chain's image views")

	mImageViews.resize(mImages.size());

	for (uint32 ImageId = 0; ImageId < mImageViews.size(); ++ImageId)
	{
		VkImageViewCreateInfo CreateInfo;
		CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		CreateInfo.image = mImages[ImageId];
		CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		CreateInfo.format = mImageFormat;

		CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		CreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		CreateInfo.subresourceRange.baseMipLevel = 0;
		CreateInfo.subresourceRange.levelCount = 1;
		CreateInfo.subresourceRange.baseArrayLayer = 0;
		CreateInfo.subresourceRange.layerCount = 1;

		const VkResult CreateImageViewResult = vkCreateImageView(Device->GetVulkanDevice(), &CreateInfo, nullptr, &mImageViews[ImageId]);
		CHECK_VULKAN_RESULT(CreateImageViewResult, "Error: {} during creation swap chain's image view");
	}
}
