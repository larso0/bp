#include <bpView/Window.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void Window::init(VkInstance instance, uint32_t width, uint32_t height, const std::string& title,
		  GLFWmonitor* monitor, const FlagSet<bp::Window::Flags>& flags)
{
	Window::instance = instance;
	Window::width = width;
	Window::height = height;
	Window::title = title;
	Window::flags = flags;

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
	glfwSetFramebufferSizeCallback(handle, windowSizeCallback);
	glfwSetDropCallback(handle, fileDropCallback);

	glfwGetCursorPos(handle, &motion.x, &motion.y);
	connect(cursorPosEvent, motion, &Motion::update);

	resize.width = width;
	resize.height = height;
	connect(sizeChangedEvent, resize, &Resize::update);
	int w, h;
	glfwGetFramebufferSize(handle, &w, &h);
	width = w;
	height = h;
}

Window::~Window()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
	glfwDestroyWindow(handle);
}

void Window::handleEvents()
{
	if (motion.updated)
	{
		cursorMotionEvent(motion.x - motion.previousX, motion.y - motion.previousY);
		motion.previousX = motion.x;
		motion.previousY = motion.y;
		motion.updated = false;
	}

	if (resize.updated)
	{
		resizeEvent(resize.width, resize.height);
		resize.updated = false;
	}
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
	if (width <= 0 || height <= 0) return;
	Window* w = static_cast<Window*>(glfwGetWindowUserPointer(handle));
	w->sizeChangedEvent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
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