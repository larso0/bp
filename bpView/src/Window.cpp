#include <bpView/Window.h>
#include <stdexcept>

using namespace std;

namespace bpView
{

Window::~Window()
{
	if (isReady())
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
		glfwDestroyWindow(handle);
	}
}

void Window::init()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

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

void Window::setInstance(VkInstance instance)
{
	if (isReady())
		throw runtime_error("Failed to alter instance, window already created.");
	this->instance = instance;
}

void Window::setMonitor(GLFWmonitor* monitor)
{
	if (isReady())
		throw runtime_error("Failed to alter monitor, window already created.");
	this->monitor = monitor;

	const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);
	width = vidMode->width;
	height = vidMode->height;
}

void Window::setSize(int w, int h)
{
	if (isReady())
		glfwSetWindowSize(handle, w, h);
	else
	{
		width = w;
		height = h;
	}
}

void Window::setTitle(const string& title)
{
	if (isReady())
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