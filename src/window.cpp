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

		create_depth_image();
		create_render_pass();
		create_framebuffers();

		VkCommandBufferAllocateInfo cmd_buffer_info = {};
		cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buffer_info.commandPool = m_device->graphics_command_pool();
		cmd_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_buffer_info.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(m_device->logical_handle(), &cmd_buffer_info,
						  &m_present_cmd_buffer);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to allocate present command buffer.");

		VkSemaphoreCreateInfo sem_info = {
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0};
		result = vkCreateSemaphore(m_device->logical_handle(), &sem_info, nullptr,
					   &m_render_complete_sem);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create render complete semaphore.");

		connect(resize_event, *this, &bp::window::on_resize);

		m_realized = true;
	}

	void window::close()
	{
		if (!m_realized) return;
		vkFreeCommandBuffers(m_device->logical_handle(), m_device->graphics_command_pool(),
				     1, &m_present_cmd_buffer);
		m_present_cmd_buffer = VK_NULL_HANDLE;
		m_swapchain.reset();
		m_device = nullptr;
		vkDestroySurfaceKHR(instance, m_surface, nullptr);
		m_surface = VK_NULL_HANDLE;
		glfwDestroyWindow(m_handle);
		m_handle = nullptr;
		m_realized = false;
	}

	void window::begin_frame()
	{
		if (m_size_changed) update_size();

		m_swapchain.next_image();

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(m_present_cmd_buffer, &begin_info);

		m_swapchain.transition_color(m_present_cmd_buffer);

		VkRenderPassBeginInfo render_pass_begin_info = {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = m_render_pass;
		render_pass_begin_info.framebuffer =
			m_framebuffers[m_swapchain.current_image_index()];
		render_pass_begin_info.renderArea = {{0, 0}, m_resolution};
		render_pass_begin_info.clearValueCount = 2;
		render_pass_begin_info.pClearValues = m_clear_values;
		vkCmdBeginRenderPass(m_present_cmd_buffer, &render_pass_begin_info,
				     VK_SUBPASS_CONTENTS_INLINE);
	}

	void window::end_frame()
	{
		vkCmdEndRenderPass(m_present_cmd_buffer);

		m_swapchain.transition_present(m_present_cmd_buffer);

		vkEndCommandBuffer(m_present_cmd_buffer);

		VkFence render_fence;
		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		vkCreateFence(m_device->logical_handle(), &fence_info, nullptr, &render_fence);

		VkSemaphore present_sem = m_swapchain.present_semaphore();

		VkPipelineStageFlags wait_stages = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &present_sem;
		submit_info.pWaitDstStageMask = &wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &m_present_cmd_buffer;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &m_render_complete_sem;
		vkQueueSubmit(m_device->graphics_queue(), 1, &submit_info, render_fence);

		vkWaitForFences(m_device->logical_handle(), 1, &render_fence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(m_device->logical_handle(), render_fence, nullptr);

		m_swapchain.present(m_render_complete_sem);
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
			m_resolution = {(uint32_t) width, (uint32_t) height};
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

	void window::create_depth_image()
	{
		m_depth_image = new image();
		m_depth_image->use_device(m_device);
		m_depth_image->set_size(m_resolution.width, m_resolution.height);
		m_depth_image->set_format(VK_FORMAT_D16_UNORM);
		m_depth_image->set_tiling(VK_IMAGE_TILING_OPTIMAL);
		m_depth_image->set_layout(VK_IMAGE_LAYOUT_UNDEFINED);
		m_depth_image->set_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		m_depth_image->set_memory_properties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_depth_image->realize();
		m_depth_image->transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		VkImageViewCreateInfo image_view_info = {};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.image = m_depth_image->handle();
		image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_info.format = VK_FORMAT_D16_UNORM;
		image_view_info.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.levelCount = 1;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(m_device->logical_handle(), &image_view_info,
						    nullptr, &m_depth_image_view);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create depth image view.");
	}

	void window::create_render_pass()
	{
		VkAttachmentDescription pass_attachments[2] = {};
		pass_attachments[0].format = m_swapchain.format();
		pass_attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		pass_attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		pass_attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		pass_attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		pass_attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		pass_attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		pass_attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		pass_attachments[1].format = VK_FORMAT_D16_UNORM;
		pass_attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		pass_attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		pass_attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		pass_attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		pass_attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		pass_attachments[1].initialLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		pass_attachments[1].finalLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment = {};
		depth_attachment.attachment = 1;
		depth_attachment.layout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;
		subpass.pDepthStencilAttachment = &depth_attachment;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 2;
		info.pAttachments = pass_attachments;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;

		VkResult result = vkCreateRenderPass(m_device->logical_handle(), &info, nullptr,
						     &m_render_pass);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create render pass.");
	}

	void window::create_framebuffers()
	{
		VkImageView attachments[2];
		attachments[1] = m_depth_image_view;

		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = m_render_pass;
		info.attachmentCount = 2;
		info.pAttachments = attachments;
		info.width = m_resolution.width;
		info.height = m_resolution.height;
		info.layers = 1;

		uint32_t n = m_swapchain.image_count();
		m_framebuffers.resize(n);
		for (uint32_t i = 0; i < n; i++)
		{
			attachments[0] = m_swapchain.image_views()[i];
			VkResult result = vkCreateFramebuffer(m_device->logical_handle(), &info,
							      nullptr, m_framebuffers.data() + i);
			if (result != VK_SUCCESS)
				throw runtime_error("Failed to create framebuffer.");
		}
	}

	void window::on_resize(int width, int height)
	{
		m_resolution = {(uint32_t) width, (uint32_t) height};
		m_size_changed = true;
	}

	void window::update_size()
	{
		vkDeviceWaitIdle(m_device->logical_handle());
		for (VkFramebuffer b : m_framebuffers)
			vkDestroyFramebuffer(m_device->logical_handle(), b, nullptr);
		vkDestroyImageView(m_device->logical_handle(), m_depth_image_view, nullptr);
		delete m_depth_image;

		m_swapchain.resize(m_resolution.width, m_resolution.height);
		create_depth_image();
		create_framebuffers();
		m_size_changed = false;
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