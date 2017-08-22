#include <bp/window.h>
#include <stdexcept>

using namespace std;

namespace bp
{
	extern VkInstance instance;

	window::~window()
	{
		close();
	}

	void window::realize()
	{
		m_handle = glfwCreateWindow(m_resolution.width, m_resolution.height,
					    m_title.c_str(), m_monitor, nullptr);
		if (!m_handle)
			throw runtime_error("Failed to create GLFW window.");

		glfwSetWindowUserPointer(m_handle, this);
		glfwSetKeyCallback(m_handle, key_callback);
		glfwSetCharCallback(m_handle, char_callback);
		glfwSetMouseButtonCallback(m_handle, mouse_btn_callback);
		glfwSetCursorPosCallback(m_handle, cursor_pos_callback);
		glfwSetCursorEnterCallback(m_handle, cursor_enter_callback);
		glfwSetWindowSizeCallback(m_handle, window_size_callback);
		glfwSetDropCallback(m_handle, file_drop_callback);

		VkResult result = glfwCreateWindowSurface(instance, m_handle, nullptr, &m_surface);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create window surface.");

		if (!m_device)
		{
			m_device = make_shared<bp::device>();
			m_device->set_queues(VK_QUEUE_GRAPHICS_BIT);
		}

		m_device->use_surface(m_surface);

		if (!m_device->is_realized()) m_device->realize();

		m_swapchain.use_device(m_device);
		m_swapchain.use_surface(m_surface);
		m_swapchain.set_size(m_resolution.width, m_resolution.height);
		m_swapchain.realize();

		connect(resize_event, m_swapchain, &bp::swapchain::resize);

		m_realized = true;
	}

	void window::close()
	{
		if (!m_realized) return;
		m_swapchain.~swapchain();
		m_swapchain = bp::swapchain();
		m_device = nullptr;
		vkDestroySurfaceKHR(instance, m_surface, nullptr);
		glfwDestroyWindow(m_handle);
		m_realized = false;
	}

	void window::use_device(std::shared_ptr<bp::device> device)
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
		if (m_realized)
			glfwSetWindowSize(m_handle, width, height);
		else
		{
			m_resolution = { (uint32_t)width, (uint32_t)height };
		}
	}

	void window::set_title(const std::string& title)
	{
		m_title = title;

		if (m_realized)
			glfwSetWindowTitle(m_handle, title.c_str());
	}

	void window::set_device_queues(VkQueueFlags queues)
	{
		if (!m_device) m_device = make_shared<bp::device>();
		m_device->set_queues(queues | VK_QUEUE_GRAPHICS_BIT);
	}

	void window::set_device_features(const VkPhysicalDeviceFeatures& features)
	{
		if (!m_device) m_device = make_shared<bp::device>();
		m_device->set_features(features);
	}

	void window::key_callback(GLFWwindow* handle, int key, int, int action, int mods)
	{
		window* w = static_cast<window*>(glfwGetWindowUserPointer(handle));
		switch (action)
		{
		case GLFW_PRESS:
			w->key_press_event(key, mods);
			break;
		case GLFW_RELEASE:
			w->key_release_event(key, mods);
			break;
		case GLFW_REPEAT:
			w->key_repeat_event(key, mods);
			break;
		default:
			break;
		}
	}

	void window::char_callback(GLFWwindow* handle, unsigned int codepoint)
	{
		window* w = static_cast<window*>(glfwGetWindowUserPointer(handle));
		w->char_input_event(codepoint);
	}

	void window::mouse_btn_callback(GLFWwindow* handle, int button, int action, int mods)
	{
		window* w = static_cast<window*>(glfwGetWindowUserPointer(handle));
		switch (action)
		{
		case GLFW_PRESS:
			w->mouse_btn_press_event(button, mods);
			break;
		case GLFW_RELEASE:
			w->mouse_btn_release_event(button, mods);
			break;
		default:
			break;
		}
	}

	void window::cursor_pos_callback(GLFWwindow* handle, double x, double y)
	{
		window* w = static_cast<window*>(glfwGetWindowUserPointer(handle));
		w->cursor_pos_event(x, y);
	}

	void window::cursor_enter_callback(GLFWwindow* handle, int entered)
	{
		window* w = static_cast<window*>(glfwGetWindowUserPointer(handle));
		if (entered) w->cursor_enter_event();
		else w->cursor_leave_event();
	}

	void window::window_size_callback(GLFWwindow* handle, int width, int height)
	{
		window* w = static_cast<window*>(glfwGetWindowUserPointer(handle));
		w->resize_event(width, height);
		w->m_resolution = { (uint32_t)width, (uint32_t)height };
	}

	void window::file_drop_callback(GLFWwindow* handle, int count, const char** c_paths)
	{
		window* w = static_cast<window*>(glfwGetWindowUserPointer(handle));
		vector<string> paths;
		for (int i = 0; i < count; i++)
			paths.push_back(string(c_paths[i]));
		w->file_drop_event(paths);
	}
}