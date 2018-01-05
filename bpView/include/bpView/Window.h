#ifndef BP_VIEW_WINDOW_H
#define BP_VIEW_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <bp/Event.h>
#include <bp/FlagSet.h>

namespace bp
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

	Window() :
		instance{VK_NULL_HANDLE},
		handle{nullptr},
		surface{VK_NULL_HANDLE},
		width{0}, height{0} {}
	Window(VkInstance instance, uint32_t width, uint32_t height, const std::string& title,
	       GLFWmonitor* monitor = nullptr, const FlagSet<Flags>& flags =
			FlagSet<Flags>() << Flags::RESIZABLE << Flags::VISIBLE
					 << Flags::DECORATED << Flags::AUTO_ICONIFY) :
		Window{}
	{
		init(instance, width, height, title, monitor, flags);
	}

	~Window();

	void init(VkInstance instance, uint32_t width, uint32_t height, const std::string& title,
		  GLFWmonitor* monitor = nullptr,
		  const FlagSet<Flags>& flags = FlagSet<Flags>()
			  << Flags::RESIZABLE << Flags::VISIBLE
			  << Flags::DECORATED << Flags::AUTO_ICONIFY);
	void handleEvents();

	void setSize(int width, int height);
	void setPosition(int x, int y);
	void setTitle(const std::string& title);

	operator VkSurfaceKHR() { return surface; }

	GLFWwindow* getHandle() { return handle; }
	VkSurfaceKHR getSurface() { return surface; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	const std::string& getTitle() const { return title; }

	Event<int, int> keyPressEvent;
	Event<int, int> keyReleaseEvent;
	Event<int, int> keyRepeatEvent;
	Event<unsigned int> charInputEvent;
	Event<int, int> mouseBtnPressEvent;
	Event<int, int> mouseBtnReleaseEvent;
	Event<double, double> cursorPosEvent;
	Event<> cursorEnterEvent;
	Event<> cursorLeaveEvent;
	Event<uint32_t, uint32_t> sizeChangedEvent;
	Event<const std::vector<std::string>&> fileDropEvent;

	Event<double, double> cursorMotionEvent;
	Event<uint32_t, uint32_t> resizeEvent;
private:
	VkInstance instance;
	GLFWwindow* handle;
	VkSurfaceKHR surface;
	uint32_t width, height;
	std::string title;
	FlagSet<Flags> flags;

	struct Motion
	{
		Motion() : updated{false}, previousX{0.0}, previousY{0.0}, x{0.0}, y{0.0} {}
		void update(double x, double y)
		{
			updated = true;
			this->x = x;
			this->y = y;
		}

		bool updated;
		double previousX, previousY;
		double x, y;
	} motion;

	struct Resize
	{
		Resize() : updated{false}, width{0}, height{0} {}
		void update(uint32_t w, uint32_t h)
		{
			updated = true;
			width = w;
			height = h;
		}

		bool updated;
		uint32_t width, height;
	} resize;

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