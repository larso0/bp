#ifndef BP_WINDOW_H
#define BP_WINDOW_H

#include "vk_includes.h"
#include "device.h"
#include "ed.h"
#include <string>
#include <memory>

namespace bp
{
	class window
	{
	public:
		window() :
			m_handle(nullptr),
			m_surface(VK_NULL_HANDLE),
			m_monitor(nullptr),
			m_width(1024),
			m_height(768),
			m_title("bp window") {}

		~window();

		void realize();

		void use_monitor(GLFWmonitor* monitor);
		void use_device(std::shared_ptr<device> device);

		void set_size(int width, int height);
		void set_title(const std::string& title);
		void set_device_queues(VkQueueFlags queues);
		void set_device_features(const VkPhysicalDeviceFeatures& features);

		bool is_realized() const { return m_realized; }
		bool should_close() const { return (bool)glfwWindowShouldClose(m_handle); }

		GLFWwindow* handle() { return m_handle; }
		const GLFWwindow* handle() const { return m_handle; }

		event<int, int> resize_event;

	private:
		bool m_realized;

		GLFWwindow* m_handle;
		VkSurfaceKHR m_surface;

		GLFWmonitor* m_monitor;
		int m_width, m_height;
		std::string m_title;

		std::shared_ptr<device> m_device;
	};
}

#endif