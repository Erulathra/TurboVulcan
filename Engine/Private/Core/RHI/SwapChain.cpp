#include "Core/RHI/SwapChain.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/Math/Math.h"
#include "Core/Math/Vector2D.h"
#include "Core/RHI/VulkanHardwareDevice.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;

FSwapChain::FSwapChain(FVulkanDevice& device)
	: mDevice(&device)
{

}

void FSwapChain::Init()
{
	TURBO_CHECK(mDevice->IsValid());

	const FVulkanHardwareDevice* hardwareDevice = mDevice->GetHardwareDevice();

	TURBO_LOG(LOG_RHI, LOG_INFO, "Creating Swap chain");

	const SwapChainDeviceSupportDetails supportDetails = hardwareDevice->QuerySwapChainSupport();

	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo.setSurface(gEngine->GetWindow()->GetVulkanSurface());
	vk::SurfaceFormatKHR pixelFormat = SelectBestSurfacePixelFormat(supportDetails.Formats);
	createInfo.setImageFormat(pixelFormat.format);
	createInfo.setImageColorSpace(pixelFormat.colorSpace);
	createInfo.setImageArrayLayers(1);
	createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
	createInfo.setPresentMode(SelectBestPresentMode(supportDetails.PresentModes));

	uint32 imageCount = supportDetails.Capabilities.minImageCount + 1;
	if (supportDetails.Capabilities.maxImageCount > 0)
	{
		imageCount = FMath::Min(imageCount, supportDetails.Capabilities.maxImageCount);
	}
	createInfo.minImageCount = imageCount;
	createInfo.imageExtent = CalculateSwapChainExtent(supportDetails.Capabilities);

	const FQueueFamilyIndices& queueIndices = mDevice->GetQueueIndices();
	const std::set<uint32> uniqueQueueIndices = queueIndices.GetUniqueQueueIndices();
	if (queueIndices.GraphicsFamily == queueIndices.PresentFamily)
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}
	else
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = queueIndices.Num();

		const std::vector queueIndicesVector(uniqueQueueIndices.begin(), uniqueQueueIndices.end());
		createInfo.setQueueFamilyIndices(queueIndicesVector);
	}

	createInfo.preTransform = supportDetails.Capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

	createInfo.presentMode = SelectBestPresentMode(supportDetails.PresentModes);
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = nullptr;

	vk::Result vulkanResult;
	std::tie(vulkanResult, mVulkanSwapChain) = mDevice->GetVulkanDevice().createSwapchainKHR(createInfo);
	CHECK_VULKAN_HPP_MSG(vulkanResult, "Cannot create swapchain");

	mImageFormat = createInfo.imageFormat;
	mImageSize = createInfo.imageExtent;

	std::tie(vulkanResult, mImages) = mDevice->GetVulkanDevice().getSwapchainImagesKHR(mVulkanSwapChain);
	InitializeImageViews();
}


void FSwapChain::Destroy()
{
	TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying Swap chain");
	TURBO_CHECK(mDevice->IsValid());

	for (vk::ImageView& view : mImageViews)
	{
		mDevice->GetVulkanDevice().destroyImageView(view);
	}
	mImageViews.clear();

	mDevice->GetVulkanDevice().destroySwapchainKHR((mVulkanSwapChain));
}

vk::SurfaceFormatKHR FSwapChain::SelectBestSurfacePixelFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
{
	TURBO_CHECK(!availableFormats.empty());

	for (const vk::SurfaceFormatKHR& format : availableFormats)
	{
		if (format.format == vk::Format::eB8G8R8A8Srgb
			&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return format;
		}
	}

	return availableFormats.front();
}

vk::PresentModeKHR FSwapChain::SelectBestPresentMode(const std::vector<vk::PresentModeKHR>& availableModes) const
{
	TURBO_CHECK(!availableModes.empty());

	if (std::ranges::contains(availableModes, vk::PresentModeKHR::eImmediate))
	{
		return vk::PresentModeKHR::eImmediate;
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D FSwapChain::CalculateSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
	FUIntVector2 FramebufferSize = gEngine->GetWindow()->GetFrameBufferSize();
	FramebufferSize.x = FMath::Clamp(FramebufferSize.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	FramebufferSize.y = FMath::Clamp(FramebufferSize.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return  {FramebufferSize.x, FramebufferSize.y};
}

void FSwapChain::InitializeImageViews()
{
	TURBO_LOG(LOG_RHI, LOG_DISPLAY, "Creating swap chain's image views")

	mImageViews.resize(mImages.size());

	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subresourceRange.setLayerCount(1);
	subresourceRange.setLevelCount(1);

	vk::ImageViewCreateInfo createInfo{};
	createInfo.setViewType(vk::ImageViewType::e2D);
	createInfo.setFormat(mImageFormat);
	createInfo.setSubresourceRange(subresourceRange);

	for (uint32 imageId = 0; imageId < mImageViews.size(); ++imageId)
	{
		createInfo.setImage(mImages[imageId]);

		vk::Result result;
		std::tie(result, mImageViews[imageId]) = mDevice->GetVulkanDevice().createImageView(createInfo);
		CHECK_VULKAN_HPP_MSG(result, "Cannot create swapchain image view");
	}
}
