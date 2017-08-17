#include <bp/bp.h>
#include <GLFW/glfw3.h>

namespace bp
{
	void poll_events()
	{
		glfwPollEvents();
	}

	void wait_events()
	{
		glfwWaitEvents();
	}

	void wait_events(double timeout)
	{
		glfwWaitEventsTimeout(timeout);
	}
}

