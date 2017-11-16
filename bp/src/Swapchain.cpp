#include <bp/Swapchain.h>
#include <bp/Util.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void Swapchain::init(NotNull<Device> device, VkSurfaceKHR surface, uint32_t width, uint32_t height,
		     const FlagSet<Flags>& flags)
{
	if (isReady()) throw runtime_error("Swapchain already initialized.");
	if (surface == VK_NULL_HANDLE) throw invalid_argument("Surface must be a valid handle.");
	this->surface = surface;
	this->flags = flags;
	RenderTarget::init(device, VK_FORMAT_B8G8R8_UNORM, width, height,
			   flags & Flags::DEPTH_IMAGE);
	framebufferImageCount = 2;
	create();
}

Swapchain::~Swapchain()
{
	for (VkImageView imageView : framebufferImageViews)
		vkDestroyImageView(*device, imageView, nullptr);
	vkDestroySwapchainKHR(*device, handle, nullptr);
}

void Swapchain::beginFrame(VkCommandBuffer cmdBuffer)
{
	assertReady();
	nextImage();
	transitionColor(cmdBuffer);
	if (flags & Flags::DEPTH_STAGING_IMAGE)
	{
		depthImage->transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
	}
}

void Swapchain::endFrame(VkCommandBuffer cmdBuffer)
{
	assertReady();
	transitionPresent(cmdBuffer);
	if (flags & Flags::DEPTH_STAGING_IMAGE)
	{
		depthImage->transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				       VK_ACCESS_TRANSFER_READ_BIT,
				       VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);
	}
}

void Swapchain::present(VkSemaphore waitSemaphore)
{
	assertReady();
	if (flags & Flags::DEPTH_STAGING_IMAGE)
	{
		VkCommandBuffer cmdBuffer = beginSingleUseCmdBuffer(*device, cmdPool);
		depthStagingImage->transfer(*depthImage, cmdBuffer);
		depthStagingImage->transition(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_HOST_READ_BIT,
					      VK_PIPELINE_STAGE_HOST_BIT, cmdBuffer);
		endSingleUseCmdBuffer(*device, device->getGraphicsQueue(), cmdPool, cmdBuffer);
	}
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &handle;
	presentInfo.pImageIndices = &currentFramebufferIndex;
	presentInfo.pResults = nullptr;
	vkQueuePresentKHR(device->getGraphicsQueue(), &presentInfo);
}

void Swapchain::resize(uint32_t w, uint32_t h)
{
	RenderTarget::resize(w, h);
	create();
}

void Swapchain::create()
{
	uint32_t n;
	vkGetPhysicalDeviceSurfaceFormatsKHR(*device, surface, &n, nullptr);
	vector<VkSurfaceFormatKHR> surfaceFormats(n);
	vkGetPhysicalDeviceSurfaceFormatsKHR(*device, surface, &n, surfaceFormats.data());

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
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*device, surface,
						  &surfaceCapabilities);

	if (framebufferImageCount < surfaceCapabilities.minImageCount)
		framebufferImageCount = surfaceCapabilities.minImageCount;
	else if (surfaceCapabilities.maxImageCount != 0 &&
		 framebufferImageCount > surfaceCapabilities.maxImageCount)
		framebufferImageCount = surfaceCapabilities.maxImageCount;

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

	vkGetPhysicalDeviceSurfacePresentModesKHR(*device, surface, &n, nullptr);
	vector<VkPresentModeKHR> present_modes(n);
	vkGetPhysicalDeviceSurfacePresentModesKHR(*device, surface, &n,
						  present_modes.data());

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (!(flags & Flags::VERTICAL_SYNC))
	{
		for (uint32_t i = 0; i < n; i++)
		{
			if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = present_modes[i];
				break;
			} else if (present_modes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR ||
				   (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR &&
				    presentMode == VK_PRESENT_MODE_FIFO_KHR))
			{
				presentMode = present_modes[i];
			}
		}
	}

	VkSwapchainKHR oldSwapchain = handle;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = framebufferImageCount;
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

	VkResult result = vkCreateSwapchainKHR(*device, &createInfo, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create swapchain.");

	if (oldSwapchain != VK_NULL_HANDLE)
	{
		for (VkImageView imageView : framebufferImageViews)
			vkDestroyImageView(*device, imageView, nullptr);
		vkDestroySwapchainKHR(*device, oldSwapchain, nullptr);
	}

	vkGetSwapchainImagesKHR(*device, handle, &n, nullptr);
	framebufferImages.resize(n);
	vkGetSwapchainImagesKHR(*device, handle, &n, framebufferImages.data());

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

	framebufferImageViews.resize(framebufferImageCount);
	for (uint32_t i = 0; i < framebufferImageCount; i++)
	{
		imageViewInfo.image = framebufferImages[i];
		result = vkCreateImageView(*device, &imageViewInfo, nullptr,
					   framebufferImageViews.data() + i);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create swapchain image view.");
	}
}

void Swapchain::nextImage()
{
	vkAcquireNextImageKHR(*device, handle, UINT64_MAX, presentSemaphore, VK_NULL_HANDLE,
			      &currentFramebufferIndex);
}

void Swapchain::transitionColor(VkCommandBuffer cmdBuffer)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	if (transitionStatus[currentFramebufferIndex])
	{
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	} else
	{
		barrier.srcAccessMask = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		transitionStatus[currentFramebufferIndex] = true;
	}
	barrier.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.newLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = framebufferImages[currentFramebufferIndex];
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
	barrier.image = framebufferImages[currentFramebufferIndex];

	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
			     &barrier);
}

}