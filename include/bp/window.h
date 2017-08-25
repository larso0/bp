#ifndef BP_WINDOW_H
#define BP_WINDOW_H

#include "vk_includes.h"
#include "device.h"
#include "ed.h"
#include "swapchain.h"
#include "image.h"
#include <string>
#include <memory>
#include <vector>

namespace bp
{
	class window
	{
	public:
		window() :
			m_handle(nullptr),
			m_surface(VK_NULL_HANDLE),
			m_monitor(nullptr),
			m_resolution({(uint32_t) 1024, (uint32_t) 768}),
			m_title("bp window"),
			m_clear_enabled(true),
			m_depth_image(nullptr),
			m_depth_image_view(VK_NULL_HANDLE),
			m_render_pass(VK_NULL_HANDLE),
			m_present_cmd_buffer(VK_NULL_HANDLE),
			m_render_complete_sem(VK_NULL_HANDLE),
			m_size_changed(false)
		{
			m_clear_values[0] = {0.5f, 0.5f, 0.5f, 1.f};
			m_clear_values[1] = {1.f, 0.f};
		}

		~window();

		void realize();
		void close();

		void begin_frame();
		void end_frame();

		void use_monitor(GLFWmonitor* monitor);
		void use_device(std::shared_ptr<bp::device> device);

		void set_size(int width, int height);
		void set_title(const std::string& title);
		void set_clear_color(VkClearColorValue color) { m_clear_values[0].color = color; }
		void set_clear_enabled(bool enabled) { m_clear_enabled = enabled; }
		void set_device_queues(VkQueueFlags queues);
		void set_device_features(const VkPhysicalDeviceFeatures& features);

		bool is_realized() const { return m_realized; }
		bool should_close() const { return (bool) glfwWindowShouldClose(m_handle); }

		GLFWwindow* handle() { return m_handle; }
		const GLFWwindow* handle() const { return m_handle; }
		VkSurfaceKHR surface() { return m_surface; }
		GLFWmonitor* monitor() const { return m_monitor; }
		VkExtent2D resolution() const { return m_resolution; }
		const std::string& title() const { return m_title; }
		std::shared_ptr<bp::device> device() { return m_device; }
		const std::shared_ptr<bp::device> device() const { return m_device; }

		event<int, int> key_press_event;
		event<int, int> key_release_event;
		event<int, int> key_repeat_event;
		event<unsigned int> char_input_event;
		event<int, int> mouse_btn_press_event;
		event<int, int> mouse_btn_release_event;
		event<double, double> cursor_pos_event;
		event<> cursor_enter_event;
		event<> cursor_leave_event;
		event<int, int> resize_event;
		event<const std::vector<std::string>&> file_drop_event;

	private:
		bool m_realized;

		GLFWwindow* m_handle;
		VkSurfaceKHR m_surface;

		GLFWmonitor* m_monitor;
		VkExtent2D m_resolution;
		std::string m_title;
		VkClearValue m_clear_values[2];
		bool m_clear_enabled;

		std::shared_ptr<bp::device> m_device;

		bp::swapchain m_swapchain;
		image* m_depth_image;
		VkImageView m_depth_image_view;
		VkRenderPass m_render_pass;
		std::vector<VkFramebuffer> m_framebuffers;
		VkCommandBuffer m_present_cmd_buffer;
		VkSemaphore m_render_complete_sem;

		void create_depth_image();
		void create_render_pass();
		void create_framebuffers();

		bool m_size_changed;
		void on_resize(int width, int height);
		void update_size();

		static void key_callback(GLFWwindow* handle, int key, int, int action, int mods);
		static void char_callback(GLFWwindow* handle, unsigned int codepoint);
		static void
		mouse_btn_callback(GLFWwindow* handle, int button, int action, int mods);
		static void cursor_pos_callback(GLFWwindow* handle, double x, double y);
		static void cursor_enter_callback(GLFWwindow* handle, int entered);
		static void window_size_callback(GLFWwindow* window, int width, int height);
		static void file_drop_callback(GLFWwindow* handle, int count, const char** paths);
	};
}

#endif