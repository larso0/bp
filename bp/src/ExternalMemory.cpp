#include <bp/ExternalMemory.h>

using namespace std;

namespace bp
{

const array<string, 2> ExternalMemory::requiredInstanceExtensions{
	"VK_KHR_get_physical_device_properties2",
	"VK_KHR_external_memory_capabilities"
};

const array<string, 2> ExternalMemory::requiredDeviceExtensions{
	"VK_KHR_external_memory",
	"VK_EXT_external_memory_host"
};

ExternalMemory::ExternalMemory(VkPhysicalDevice physicalDevice, VkDevice device,
			       const VkMemoryRequirements& requirements, void* opaque) :
	device{device},
	handle{VK_NULL_HANDLE},
	size{requirements.size},
	opaque{opaque}
{
	VkMemoryAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	info.allocationSize = size;

	PFN_vkGetMemoryHostPointerPropertiesEXT vkGetMemoryHostPointerPropertiesEXT =
		static_cast<PFN_vkGetMemoryHostPointerPropertiesEXT>(
			vkGetDeviceProcAddr(device, "vkGetMemoryHostPointerPropertiesEXT"));
}

ExternalMemory::~ExternalMemory()
{
}

}