#include <bp/window.h>
#include <stdexcept>

using namespace std;

namespace bp
{
	extern VkInstance instance;

	window::~window()
	{
		if (!m_realized) return;

		vkDestroySurfaceKHR(instance, m_surface, nullptr);
		glfwDestroyWindow(m_handle);
	}

	void window::realize()
	{
		m_handle = glfwCreateWindow(m_width, m_height, m_title.c_str(), m_monitor, nullptr);
		if (!m_handle)
			throw runtime_error("Failed to create GLFW window.");

		glfwSetWindowUserPointer(m_handle, this);
		//TODO set callbacks

		VkResult result = glfwCreateWindowSurface(instance, m_handle, nullptr, &m_surface);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create window surface.");

		if (!m_device)
		{
			m_device = make_shared<device>();
			m_device->set_queues(VK_QUEUE_GRAPHICS_BIT);
		}

		m_device->use_surface(m_surface);

		if (!m_device->is_realized()) m_device->realize();
	}

	void window::use_device(std::shared_ptr<device> device)
	{
		if (m_realized)
			throw runtime_error("Failed to alter device, window is already realized.");

		m_device = device;
	}

	void window::use_monitor(GLFWmonitor* monitor)
	{
		if (m_realized)
			throw runtime_error("Failed to alter monitor, window is already realized.");

		m_monitor = monitor;
	}

	void window::set_size(int width, int height)
	{
		m_width = width;
		m_height = height;

		if (m_realized)
			glfwSetWindowSize(m_handle, width, height);
	}

	void window::set_title(const std::string& title)
	{
		m_title = title;

		if (m_realized)
			glfwSetWindowTitle(m_handle, title.c_str());
	}

	void window::set_device_queues(VkQueueFlags queues)
	{
		if (!m_device) m_device = make_shared<device>();
		m_device->set_queues(queues | VK_QUEUE_GRAPHICS_BIT);
	}

	void window::set_device_features(const VkPhysicalDeviceFeatures& features)
	{
		if (!m_device) m_device = make_shared<device>();
		m_device->set_features(features);
	}

}