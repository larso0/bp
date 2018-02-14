#include <bp/ExternalMemory.h>
#include <bp/Util.h>
#include <stdexcept>

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
		reinterpret_cast<PFN_vkGetMemoryHostPointerPropertiesEXT>(
			vkGetDeviceProcAddr(device, "vkGetMemoryHostPointerPropertiesEXT"));

	VkMemoryHostPointerPropertiesEXT properties = {};
	vkGetMemoryHostPointerPropertiesEXT(device,
					    VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT,
					    opaque, &properties);

	int32_t memType = bp::findPhysicalDeviceMemoryType(physicalDevice,
							   properties.memoryTypeBits, 0);
	if (memType == -1)
		throw runtime_error("No suitable memory type found.");

	info.memoryTypeIndex = static_cast<uint32_t>(memType);

	VkImportMemoryHostPointerInfoEXT import = {};
	import.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
	import.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
	import.pHostPointer = opaque;

	info.pNext = &import;

	VkResult result = vkAllocateMemory(device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate memory.");
}

ExternalMemory::~ExternalMemory()
{
	if (handle != VK_NULL_HANDLE)
		vkFreeMemory(device, handle, nullptr);
}

}