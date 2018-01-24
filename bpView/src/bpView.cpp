#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <bpView/bpView.h>
#include <stdexcept>
#include <sstream>

using namespace std;

namespace bpView
{

static vector<string> _requiredInstanceExtensions;
const vector<string>& requiredInstanceExtensions = _requiredInstanceExtensions;
bpUtil::Event<const std::string&> errorEvent;

static void glfwErrorCallback(int, const char* msg)
{
	stringstream ss;
	ss << "GLFW error: " << msg;
	errorEvent(ss.str());
}

void init()
{
	if (!glfwInit())
		throw runtime_error("Failed to initialized GLFW.");

	glfwSetErrorCallback(glfwErrorCallback);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (uint32_t i = 0; i < glfwExtensionCount; i++)
		_requiredInstanceExtensions.push_back(string(glfwExtensions[i]));

	atexit(glfwTerminate);
}

void pollEvents()
{
	glfwPollEvents();
}

void waitEvents()
{
	glfwWaitEvents();
}

}