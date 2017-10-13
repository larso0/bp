#include <bpView/Window.h>
#include <stdexcept>

using namespace std;

namespace bpView
{

Window::Window(VkInstance instance, uint32_t width, uint32_t height, const std::string& title,
	       GLFWmonitor* monitor, const bp::FlagSet<bpView::Window::Flags>& flags) :
	instance{instance}
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, flags & Flags::RESIZABLE);
	glfwWindowHint(GLFW_VISIBLE, flags & Flags::VISIBLE);
	glfwWindowHint(GLFW_DECORATED, flags & Flags::DECORATED);
	glfwWindowHint(GLFW_AUTO_ICONIFY, flags & Flags::AUTO_ICONIFY);
	glfwWindowHint(GLFW_FLOATING, flags & Flags::FLOATING);
	glfwWindowHint(GLFW_MAXIMIZED, flags & Flags::MAXIMIZED);

	handle = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);
	if (handle == nullptr)
		throw runtime_error("Failed to create window.");

	VkResult result = glfwCreateWindowSurface(instance, handle, nullptr, &surface);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create window surface.");

	glfwSetWindowUserPointer(handle, this);
	glfwSetKeyCallback(handle, keyCallback);
	glfwSetCharCallback(handle, charCallback);
	glfwSetMouseButtonCallback(handle, mouseBtnCallback);
	glfwSetCursorPosCallback(handle, cursorPosCallback);
	glfwSetCursorEnterCallback(handle, cursorEnterCallback);
	glfwSetWindowSizeCallback(handle, windowSizeCallback);
	glfwSetDropCallback(handle, fileDropCallback);
}

Window::~Window()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
	glfwDestroyWindow(handle);
}

void Window::setSize(int w, int h)
{
	glfwSetWindowSize(handle, w, h);
}

void Window::setPosition(int x, int y)
{
	glfwSetWindowPos(handle, x, y);
}

void Window::setTitle(const string& title)
{
	glfwSetWindowTitle(handle, title.c_str());
	this->title = title;
}

void Window::keyCallback(GLFWwindow* handle, int key, int, int action, int mods)
{
	Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
	switch (action)
	{
	case GLFW_PRESS:
		w->keyPressEvent(key, mods);
		break;
	case GLFW_RELEASE:
		w->keyReleaseEvent(key, mods);
		break;
	case GLFW_REPEAT:
		w->keyRepeatEvent(key, mods);
		break;
	default:
		break;
	}
}

void Window::charCallback(GLFWwindow* handle, unsigned int codepoint)
{
	Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
	w->charInputEvent(codepoint);
}

void Window::mouseBtnCallback(GLFWwindow* handle, int button, int action, int mods)
{
	Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
	switch (action)
	{
	case GLFW_PRESS:
		w->mouseBtnPressEvent(button, mods);
		break;
	case GLFW_RELEASE:
		w->mouseBtnReleaseEvent(button, mods);
		break;
	default:
		break;
	}
}

void Window::cursorPosCallback(GLFWwindow* handle, double x, double y)
{
	Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
	w->cursorPosEvent(x, y);
}

void Window::cursorEnterCallback(GLFWwindow* handle, int entered)
{
	Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
	if (entered) w->cursorEnterEvent();
	else w->cursorLeaveEvent();
}

void Window::windowSizeCallback(GLFWwindow* handle, int width, int height)
{
	Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
	w->resizeEvent(width, height);
}

void Window::fileDropCallback(GLFWwindow* handle, int count, const char** c_paths)
{
	Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
	vector <string> paths;
	for (int i = 0; i < count; i++)
		paths.push_back(string(c_paths[i]));
	w->fileDropEvent(paths);
}

}