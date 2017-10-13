#ifndef BP_VIEW_WINDOW_H
#define BP_VIEW_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <bp/Event.h>
#include <bp/FlagSet.h>

namespace bpView
{

class Window
{
public:
	enum class Flags : size_t
	{
		RESIZABLE,
		VISIBLE,
		DECORATED,
		AUTO_ICONIFY,
		FLOATING,
		MAXIMIZED,
		BP_FLAGSET_LAST
	};

	Window(VkInstance instance, uint32_t width, uint32_t height, const std::string& title,
	       GLFWmonitor* monitor = nullptr, const bp::FlagSet<Flags>& flags =
			bp::FlagSet<Flags>() << Flags::RESIZABLE << Flags::VISIBLE
					     << Flags::DECORATED << Flags::AUTO_ICONIFY);
	~Window();

	void setSize(int width, int height);
	void setPosition(int x, int y);
	void setTitle(const std::string& title);

	GLFWwindow* getHandle() { return handle; }
	VkSurfaceKHR getSurface() { return surface; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	const std::string& getTitle() const { return title; }

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
	GLFWwindow* handle;
	VkSurfaceKHR surface;
	int width, height;
	std::string title;
	bp::FlagSet<Flags> flags;

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