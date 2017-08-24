#ifndef BP_SWAPCHAIN_H
#define BP_SWAPCHAIN_H

#include "vk_includes.h"
#include "device.h"
#include <memory>
#include <vector>

namespace bp
{
	class swapchain
	{
	public:
		swapchain() :
			m_realized(false),
			m_handle(VK_NULL_HANDLE),
			m_device(std::shared_ptr<bp::device>()),
			m_surface(VK_NULL_HANDLE),
			m_format(VK_FORMAT_B8G8R8_UNORM),
			m_color_space(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR),
			m_resolution({}),
			m_image_count(2),
			m_present_semaphore(VK_NULL_HANDLE),
			m_current_image(0) {}
		~swapchain();

		void realize();
		void reset();
		void resize(int width, int height);

		void next_image();
		void transition_color(VkCommandBuffer cmd_buffer);
		void transition_present(VkCommandBuffer cmd_buffer);
		void present(VkSemaphore wait_sem);

		void use_device(std::shared_ptr<device> device);
		void use_surface(VkSurfaceKHR surface);

		void set_size(int width, int height);

		bool is_realized() const { return m_realized; }

		VkSwapchainKHR handle() { return m_handle; }
		VkFormat format() const { return m_format; }
		VkColorSpaceKHR color_space() const { return m_color_space; }
		VkExtent2D resolution() const { return m_resolution; }
		uint32_t image_count() const { return m_image_count; }
		const std::vector<VkImageView>& image_views() const { return m_image_views; }
		VkSemaphore present_semaphore() { return m_present_semaphore; }
		uint32_t current_image_index() const { return m_current_image; }

	private:
		bool m_realized;

		VkSwapchainKHR m_handle;

		std::shared_ptr<device> m_device;
		VkSurfaceKHR m_surface;

		VkFormat m_format;
		VkColorSpaceKHR m_color_space;
		VkExtent2D m_resolution;

		uint32_t m_image_count;
		std::vector<VkImage> m_images;
		std::vector<bool> m_transition_status;
		std::vector<VkImageView> m_image_views;
		VkSemaphore m_present_semaphore;
		uint32_t m_current_image;
	};
}

#endif