#include <bp/Util.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <thread>
#include <cstring>
#include <future>

using namespace std;

namespace bp
{

int32_t findQueueFamilyIndex(VkPhysicalDevice device, VkQueueFlags flags)
{
	uint32_t n;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, NULL);
	vector<VkQueueFamilyProperties> queueFamilyProperties(n);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, queueFamilyProperties.data());

	int32_t idx = -1;
	for (uint32_t i = 0; i < n; i++)
	{
		if (queueFamilyProperties[i].queueFlags == flags)
		{
			/*
			 * Assume that a queue family that matches exactly is more efficient.
			 * For instance, a queue family that support only transfer queues, might be
			 * more efficient at transfer than a graphics- or compute queue family
			 * (which also support transfer).
			 */
			idx = i;
			break;
		} else if ((flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT) &&
			   !(queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
			   (queueFamilyProperties[i].queueFlags & flags) == flags)
		{
			/*
			 * Assume that a graphics capable queue family that does not support compute
			 * is more efficient at graphics than a queue family that also support
			 * compute.
			 */
			idx = i;
		} else if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) &&
			   !(queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			   (queueFamilyProperties[i].queueFlags & flags) == flags)
		{
			/*
			 * Assume that a compute capable queue family that does not support graphics
			 * is more efficient at compute than a queue family that also support
			 * graphics.
			 */
			idx = i;
		} else if (!(flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) &&
			   !(queueFamilyProperties[i].queueFlags &
			     (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) &&
			   (queueFamilyProperties[i].queueFlags & flags) == flags)
		{
			/*
			 * If graphics or compute is not required, prefer queue families that does
			 * not support graphics or compute.
			 */
			idx = i;
		} else if (idx == -1 && (queueFamilyProperties[i].queueFlags & flags) == flags)
		{
			idx = i;
		}
	}

	return idx;
}

int32_t findSurfaceQueueFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t n;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, NULL);
	vector<VkQueueFamilyProperties> queueFamilyProperties(n);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, queueFamilyProperties.data());

	int32_t idx = -1;
	for (uint32_t i = 0; i < n; i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			VkBool32 surfaceSupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &surfaceSupported);
			if (surfaceSupported)
			{
				idx = i;
				break;
			}
		}
	}

	return idx;
}

int32_t findPhysicalDeviceMemoryType(VkPhysicalDevice physicalDevice, uint32_t memoryTypeBits,
				     VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		if ((memoryTypeBits & (1 << i)) &&
		    (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	return -1;
}

VkCommandBuffer beginSingleUseCmdBuffer(VkDevice device, VkCommandPool pool)
{
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = pool;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;
	vkAllocateCommandBuffers(device, &allocateInfo, &cmdBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	return cmdBuffer;
}

void endSingleUseCmdBuffer(VkDevice device, VkQueue queue, VkCommandPool pool,
			   VkCommandBuffer cmdBuffer)
{
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, pool, 1, &cmdBuffer);
}

vector<char> readBinaryFile(const string& path)
{
	ifstream file(path, ios::ate | ios::binary);

	if (!file.is_open())
	{
		stringstream ss;
		ss << "Failed to open file \"" << path << "\".";
		throw runtime_error(ss.str());
	}

	size_t fileSize = (size_t) file.tellg();
	vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

string readTextFile(const string& path)
{
	ifstream file(path);

	string str;

	file.seekg(0, ios::end);
	str.reserve(file.tellg());
	file.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(file)),
		   istreambuf_iterator<char>());

	return str;
}

void replaceSubstrings(std::string& str, const std::string& find, const std::string& replaced)
{
	std::string::size_type n = 0;
	while ((n = str.find(find, n)) != std::string::npos)
	{
		str.replace(n, find.size(), replaced);
		n += replaced.size();
	}
}

static const size_t CHUNK_SIZE = 524288;

void* parallelCopy(void* dest, const void* src, size_t count)
{
	if (count <= CHUNK_SIZE) return memmove(dest, src, count);

	size_t threadCount = thread::hardware_concurrency();
	size_t chunkSize = count / threadCount;
	if (chunkSize < CHUNK_SIZE) chunkSize = CHUNK_SIZE;
	threadCount = count / chunkSize;

	vector<future<void>> futures;
	futures.reserve(threadCount);
	for (auto i = 0; i < threadCount; i++)
		futures.push_back(async(launch::async, [dest, src, i, chunkSize]{
			memmove(static_cast<char*>(dest) + i * chunkSize,
					static_cast<const char*>(src) + i * chunkSize, chunkSize);
		}));

	size_t amountScheduled = threadCount * chunkSize;
	if (amountScheduled < count)
		memmove(static_cast<char*>(dest) + amountScheduled,
				static_cast<const char*>(src) + amountScheduled, count - amountScheduled);

	for (auto& future : futures) future.wait();

	return dest;
}

}