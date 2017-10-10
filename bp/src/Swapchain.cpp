#include <bp/Swapchain.h>
#include <stdexcept>

using namespace std;

namespace bp
{

Swapchain::~Swapchain()
{
	if (isReady())
	{
		for (VkImageView imageView : imageViews)
			vkDestroyImageView(device, imageView, nullptr);
		vkDestroySwapchainKHR(device, handle, nullptr);
	}
}

void Swapchain::init()
{
	uint32_t n;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &n, nullptr);
	vector<VkSurfaceFormatKHR> surfaceFormats(n);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &n, surfaceFormats.data());

	if (n == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		format = VK_FORMAT_B8G8R8_UNORM;
		colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	} else
	{
		uint32_t i;
		for (i = 0;
		     i < n && surfaceFormats[i].format != VK_FORMAT_B8G8R8A8_UNORM; i++);
		if (i == n) i = 0;
		format = surfaceFormats[i].format;
		colorSpace = surfaceFormats[i].colorSpace;
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
						  &surfaceCapabilities);

	if (imageCount < surfaceCapabilities.minImageCount)
		imageCount = surfaceCapabilities.minImageCount;
	else if (surfaceCapabilities.maxImageCount != 0 &&
		 imageCount > surfaceCapabilities.maxImageCount)
		imageCount = surfaceCapabilities.maxImageCount;

	VkExtent2D surfaceExtent = surfaceCapabilities.currentExtent;
	if (surfaceExtent.width == 0xFFFFFFFF || surfaceExtent.width == 0)
	{
		surfaceExtent.width = width;
		surfaceExtent.height = height;
	} else
	{
		width = surfaceExtent.width;
		height = surfaceExtent.height;
	}

	VkSurfaceTransformFlagBitsKHR preTransform =
		surfaceCapabilities.currentTransform;
	if (surfaceCapabilities.supportedTransforms &
	    VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &n, nullptr);
	vector<VkPresentModeKHR> present_modes(n);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &n,
						  present_modes.data());

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0; i < n; i++)
	{
		if ((enableVSync && present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) ||
		    (!enableVSync && present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			presentMode = present_modes[i];
			break;
		}
	}

	VkSwapchainKHR oldSwapchain = handle;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = format;
	createInfo.imageColorSpace = colorSpace;
	createInfo.imageExtent = surfaceExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = preTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = oldSwapchain;

	VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create swapchain.");

	if (oldSwapchain != VK_NULL_HANDLE)
	{
		for (VkImageView imageView : imageViews)
			vkDestroyImageView(device, imageView, nullptr);
		vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
	} else
	{
		RenderTarget::init();
	}

	vkGetSwapchainImagesKHR(device, handle, &n, nullptr);
	images.resize(n);
	vkGetSwapchainImagesKHR(device, handle, &n, images.data());

	transitionStatus = vector<bool>(n, false);

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = format;
	imageViewInfo.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	imageViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		imageViewInfo.image = images[i];
		result = vkCreateImageView(device, &imageViewInfo, nullptr,
					   imageViews.data() + i);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create swapchain image view.");
	}
}

void Swapchain::beginFrame(VkCommandBuffer cmdBuffer)
{
	nextImage();
	transitionColor(cmdBuffer);
}

void Swapchain::endFrame(VkCommandBuffer cmdBuffer)
{
	transitionPresent(cmdBuffer);
}

void Swapchain::present(VkSemaphore waitSemaphore)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &handle;
	presentInfo.pImageIndices = &currentImageIndex;
	presentInfo.pResults = nullptr;
	vkQueuePresentKHR(graphicsQueue, &presentInfo);
}

void Swapchain::resize(uint32_t w, uint32_t h)
{
	width = w;
	height = h;
	init();
	if (depthImageEnabled)
	{
		vkDestroyImageView(device, depthImageView, nullptr);
		delete depthImage;
		createDepthImage();
	}
}

void Swapchain::setSurface(VkSurfaceKHR surface)
{
	if (isReady())
		throw runtime_error("Failed to alter surface, swapchain is already created.");
	this->surface = surface;
}

void Swapchain::setVSyncEnabled(bool enabled)
{
	if (isReady())
		throw runtime_error("Failed to alter present mode, swapchain is already created.");
	enableVSync = enabled;
}

void Swapchain::nextImage()
{
	vkAcquireNextImageKHR(device, handle, UINT64_MAX, presentSemaphore, VK_NULL_HANDLE,
			      &currentImageIndex);
}

void Swapchain::transitionColor(VkCommandBuffer cmdBuffer)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	if (transitionStatus[currentImageIndex])
	{
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	} else
	{
		barrier.srcAccessMask = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		transitionStatus[currentImageIndex] = true;
	}
	barrier.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.newLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = images[currentImageIndex];
	VkImageSubresourceRange resourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	barrier.subresourceRange = resourceRange;

	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0,
			     nullptr, 1, &barrier);
}

void Swapchain::transitionPresent(VkCommandBuffer cmdBuffer)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	barrier.image = images[currentImageIndex];

	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
			     &barrier);
}

}