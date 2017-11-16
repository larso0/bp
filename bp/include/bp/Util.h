#ifndef BP_UTIL_H
#define BP_UTIL_H

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace bp
{

/*
 * Find the index of a queue family that matches the flags.
 * Returns a valid queue family index, or -1 if no suitable queue family was found.
 */
int32_t findQueueFamilyIndex(VkPhysicalDevice device, VkQueueFlags flags);

/*
 * Find the index of a queue family that support graphics queues and is capabale of presenting to
 * the given surface.
 * Returns a valid queue family index, or -1 if no suitable queue family was found.
 */
int32_t findSurfaceQueueFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface);

/*
 * Find the index of a memory type for the physical device.
 * memoryTypeBits is a bitmask of which memory type indices that should be considered. This is
 * usually VkMemoryRequirements::memoryTypeBits, after querying for memory requirements for a
 * buffer or an image.
 * properties is the memory properties that must be supported by the resulting index.
 * Returns a valid memory type index, or -1 if none were found.
 */
int32_t findPhysicalDeviceMemoryType(VkPhysicalDevice physicalDevice, uint32_t memoryTypeBits,
				     VkMemoryPropertyFlags properties);

/*
 * Allocate and begin a command buffer that should only be used once.
 * You can record commands to the returned command buffer.
 * After done recording, end the command buffer with the endSingleUseCmdBuffer function.
 */
VkCommandBuffer beginSingleUseCmdBuffer(VkDevice device, VkCommandPool pool);

/*
 * End command buffer, submit command buffer to the queue and free the command buffer.
 * Use this to end command buffer created by the function beginSingleUseCmdBuffer.
 */
void endSingleUseCmdBuffer(VkDevice device, VkQueue queue, VkCommandPool pool,
			   VkCommandBuffer cmdBuffer);

/*
 * Read a binary file into a vector.
 * Useful for loading SPIR-V binary code from files.
 */
std::vector<char> readBinaryFile(const std::string& path);

/*
 * Read a text file into a string.
 */
std::string readTextFile(const std::string& path);

/*
 * Find all occurences of find in str and replace them with replaced.
 */
void replaceSubstrings(std::string& str, const std::string& find, const std::string& replaced);

/*
 * Copy memory in parallel.
 * Utilize multi-threaded CPUs for copying memory to exploit available memory bandwidth.
 * Usage: same as memcpy
 */
void* parallelCopy(void* dest, const void* src, size_t count);

}

#endif