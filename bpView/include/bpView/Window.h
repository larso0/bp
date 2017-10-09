#ifndef BP_VIEW_WINDOW_H
#define BP_VIEW_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <bp/Event.h>

namespace bpView
{

class Window
{
public:
	Window() :
		instance(VK_NULL_HANDLE),
		monitor(nullptr),
		handle(nullptr),
		surface(VK_NULL_HANDLE),
		width(1024), height(768),
		title("Window") {}
	~Window();

	void init();

	void setInstance(VkInstance instance);
	void setMonitor(GLFWmonitor* monitor);
	void setSize(int width, int height);
	void setTitle(const std::string& title);

	GLFWwindow* getHandle() { return handle; }
	VkSurfaceKHR getSurface() { return surface; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	const std::string& getTitle() const { return title; }
	bool isReady() const { return handle != nullptr; }

	bp::Event<int, int> keyPressEvent;
	bp::Event<int, int> keyReleaseEvent;
	bp::Event<int, int> keyRepeatEvent;
	bp::Event<unsigned int> charInputEvent;
	bp::Event<int, int> mouseBtnPressEvent;
	bp::Event<int, int> mouseBtnReleaseEvent;
	bp::Event<double, double> cursorPosEvent;
	bp::Event<> cursorEnterEvent;
	bp::Event<> cursorLeaveEvent;
	bp::Event<int, int> resizeEvent;
	bp::Event<const std::vector<std::string>&> fileDropEvent;
private:
	VkInstance instance;
	GLFWmonitor* monitor;
	GLFWwindow* handle;
	VkSurfaceKHR surface;
	int width, height;
	std::string title;

	static void keyCallback(GLFWwindow* handle, int key, int, int action, int mods);
	static void charCallback(GLFWwindow* handle, unsigned int codepoint);
	static void mouseBtnCallback(GLFWwindow* handle, int button, int action, int mods);
	static void cursorPosCallback(GLFWwindow* handle, double x, double y);
	static void cursorEnterCallback(GLFWwindow* handle, int entered);
	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void fileDropCallback(GLFWwindow* handle, int count, const char** paths);
};

}

#endif