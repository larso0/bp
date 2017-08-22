#include <bp/swapchain.h>
#include <stdexcept>

using namespace std;

namespace bp
{
	swapchain::~swapchain()
	{
		if (m_realized)
		{
			vkDestroySemaphore(m_device->logical_handle(), m_present_semaphore,
					   nullptr);
			for (VkImageView i : m_image_views)
				vkDestroyImageView(m_device->logical_handle(), i, nullptr);
			vkDestroySwapchainKHR(m_device->logical_handle(), m_handle, nullptr);
		}
	}

	void swapchain::realize()
	{
		if (!m_device)
			throw runtime_error("Failed to realize swapchain, no device set.");
		if (m_surface == VK_NULL_HANDLE)
			throw runtime_error("Failed to realize swapchain, no surface set.");

		uint32_t n = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->physical_handle(), m_surface, &n,
						     nullptr);
		vector<VkSurfaceFormatKHR> surface_formats(n);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->physical_handle(), m_surface, &n,
						     surface_formats.data());

		if (n == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED)
		{
			m_format = VK_FORMAT_B8G8R8_UNORM;
			m_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		} else
		{
			uint32_t i;
			for (i = 0;
			     i < n && surface_formats[i].format != VK_FORMAT_B8G8R8A8_UNORM; i++);
			if (i == n) i = 0;
			m_format = surface_formats[i].format;
			m_color_space = surface_formats[i].colorSpace;
		}

		VkSurfaceCapabilitiesKHR surface_capabilities = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->physical_handle(), m_surface,
							  &surface_capabilities);

		if (m_image_count < surface_capabilities.minImageCount)
		{
			m_image_count = surface_capabilities.minImageCount;
		} else if (surface_capabilities.maxImageCount != 0 &&
			   m_image_count > surface_capabilities.maxImageCount)
		{
			m_image_count = surface_capabilities.maxImageCount;
		}

		VkExtent2D surface_extent = surface_capabilities.currentExtent;
		if (surface_extent.width == 0xFFFFFFFF)
		{
			m_resolution = surface_extent;
		}

		VkSurfaceTransformFlagBitsKHR pre_transform =
			surface_capabilities.currentTransform;
		if (surface_capabilities.supportedTransforms &
		    VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}

		vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->physical_handle(), m_surface,
							  &n,
							  nullptr);
		vector<VkPresentModeKHR> present_modes(n);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->physical_handle(), m_surface,
							  &n,
							  present_modes.data());

		VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
		for (uint32_t i = 0; i < n; i++)
		{
			if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
		}

		VkSwapchainKHR old_swapchain = m_handle;

		VkSwapchainCreateInfoKHR create_info;

		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.surface = m_surface;
		create_info.minImageCount = m_image_count;
		create_info.imageFormat = m_format;
		create_info.imageColorSpace = m_color_space;
		create_info.imageExtent = m_resolution;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;
		create_info.preTransform = pre_transform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = old_swapchain;

		VkResult result = vkCreateSwapchainKHR(m_device->logical_handle(), &create_info,
						       nullptr, &m_handle);
		if (result != VK_SUCCESS)
		{
			throw runtime_error("Failed to create swapchain.");
		}

		if (m_realized)
		{
			for (VkImageView i : m_image_views)
			{
				vkDestroyImageView(m_device->logical_handle(), i, nullptr);
			}
			vkDestroySwapchainKHR(m_device->logical_handle(), old_swapchain, nullptr);
		}

		vkGetSwapchainImagesKHR(m_device->logical_handle(), m_handle, &n, nullptr);
		m_images.resize(n);
		vkGetSwapchainImagesKHR(m_device->logical_handle(), m_handle, &n, m_images.data());

		m_transition_status = vector<bool>(n, false);

		VkImageViewCreateInfo image_view_info = {};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_info.format = m_format;
		image_view_info.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.levelCount = 1;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.layerCount = 1;

		m_image_views.resize(m_image_count);
		for (uint32_t i = 0; i < m_image_count; i++)
		{
			image_view_info.image = m_images[i];
			VkResult result = vkCreateImageView(m_device->logical_handle(),
							    &image_view_info,
							    nullptr, m_image_views.data() + i);
			if (result != VK_SUCCESS)
			{
				throw runtime_error("Failed to create swapchain image view.");
			}
		}

		VkSemaphoreCreateInfo sem_info = {
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			0, 0
		};
		if (!m_realized)
		{
			vkCreateSemaphore(m_device->logical_handle(), &sem_info, nullptr,
					  &m_present_semaphore);
		}

		m_realized = true;
	}

	void swapchain::resize(int width, int height)
	{
		if (!m_realized)
		{
			throw runtime_error(
				"Failed to resize swapchain, swapchain is not realized.");
		}
		set_size(width, height);
		realize();
	}

	void swapchain::next_image()
	{
		vkAcquireNextImageKHR(m_device->logical_handle(), m_handle, UINT64_MAX,
				      m_present_semaphore, VK_NULL_HANDLE, &m_current_image);
	}

	void swapchain::transition_color(VkCommandBuffer cmd_buffer)
	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		if (m_transition_status[m_current_image])
		{
			barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		} else
		{
			barrier.srcAccessMask = 0;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			m_transition_status[m_current_image] = true;
		}
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
					| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.newLayout =
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_images[m_current_image];
		VkImageSubresourceRange resourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
							 0, 1};
		barrier.subresourceRange = resourceRange;

		vkCmdPipelineBarrier(cmd_buffer,
				     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
				     0, nullptr, 0, nullptr, 1, &barrier);
	}

	void swapchain::transition_present(VkCommandBuffer cmd_buffer)
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
		barrier.image = m_images[m_current_image];

		vkCmdPipelineBarrier(cmd_buffer,
				     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
				     nullptr, 1,
				     &barrier);
	}

	void swapchain::present(VkSemaphore wait_sem)
	{
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &wait_sem;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &m_handle;
		present_info.pImageIndices = &m_current_image;
		present_info.pResults = nullptr;
		vkQueuePresentKHR(m_device->graphics_queue(), &present_info);
	}

	void swapchain::use_device(std::shared_ptr<device> device)
	{
		if (m_realized)
			throw runtime_error("Failed to alter device, swapchain already realized.");
		m_device = device;
	}

	void swapchain::use_surface(VkSurfaceKHR surface)
	{
		if (m_realized)
			throw runtime_error("Failed to alter surface, swapchain already realized.");
		m_surface = surface;
	}

	void swapchain::set_size(int width, int height)
	{
		m_resolution.width = width;
		m_resolution.height = height;
	}
}