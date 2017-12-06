#include <bpQt/DeviceQuery.h>

using namespace std;
using namespace bp;

namespace bpQt
{

vector<VkPhysicalDevice> queryDevices(const QVulkanInstance& instance,
				      const DeviceRequirements& requirements)
{
	vector<VkPhysicalDevice> results;

	uint32_t n = 0;
	vkEnumeratePhysicalDevices(instance.vkInstance(), &n, nullptr);
	vector<VkPhysicalDevice> physicalDevices(n);
	vkEnumeratePhysicalDevices(instance.vkInstance(), &n, physicalDevices.data());

	for (VkPhysicalDevice device : physicalDevices)
	{
		if (queryDevice(device, requirements))
			results.push_back(device);
	}

	return results;
}

}