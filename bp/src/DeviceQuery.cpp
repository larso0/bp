#include <bp/DeviceQuery.h>
#include <cstring>

using namespace std;

namespace bp
{

void DeviceQuery::setPhysicalDevices(uint32_t count, VkPhysicalDevice* devices)
{
	results.clear();
	physicalDeviceCount = count;
	physicalDevices = devices;
}

void DeviceQuery::setQueues(VkQueueFlags queues)
{
	results.clear();
	this->queues = queues;
}

void DeviceQuery::setFeatures(VkPhysicalDeviceFeatures* features)
{
	results.clear();
	this->features = features;
}

void DeviceQuery::setSurface(VkSurfaceKHR surface)
{
	results.clear();
	this->surface = surface;
}

void DeviceQuery::addExtension(const char* ext)
{
	results.clear();
	extensions.push_back(ext);
}

static bool deviceFeatureIncludes(const VkPhysicalDeviceFeatures* a,
				  const VkPhysicalDeviceFeatures* b)
{
	return (!a->robustBufferAccess || b->robustBufferAccess) &&
	       (!a->fullDrawIndexUint32 || b->fullDrawIndexUint32) &&
	       (!a->imageCubeArray || b->imageCubeArray) &&
	       (!a->independentBlend || b->independentBlend) &&
	       (!a->geometryShader || b->geometryShader) &&
	       (!a->tessellationShader || b->tessellationShader) &&
	       (!a->sampleRateShading || b->sampleRateShading) &&
	       (!a->dualSrcBlend || b->dualSrcBlend) &&
	       (!a->logicOp || b->logicOp) &&
	       (!a->multiDrawIndirect || b->multiDrawIndirect) &&
	       (!a->drawIndirectFirstInstance || b->drawIndirectFirstInstance) &&
	       (!a->depthClamp || b->depthClamp) &&
	       (!a->depthBiasClamp || b->depthBiasClamp) &&
	       (!a->fillModeNonSolid || b->fillModeNonSolid) &&
	       (!a->depthBounds || b->depthBounds) &&
	       (!a->wideLines || b->wideLines) &&
	       (!a->largePoints || b->largePoints) &&
	       (!a->alphaToOne || b->alphaToOne) &&
	       (!a->multiViewport || b->multiViewport) &&
	       (!a->samplerAnisotropy || b->samplerAnisotropy) &&
	       (!a->textureCompressionETC2 || b->textureCompressionETC2) &&
	       (!a->textureCompressionASTC_LDR || b->textureCompressionASTC_LDR) &&
	       (!a->textureCompressionBC || b->textureCompressionBC) &&
	       (!a->occlusionQueryPrecise || b->occlusionQueryPrecise) &&
	       (!a->pipelineStatisticsQuery || b->pipelineStatisticsQuery) &&
	       (!a->vertexPipelineStoresAndAtomics || b->vertexPipelineStoresAndAtomics) &&
	       (!a->fragmentStoresAndAtomics || b->fragmentStoresAndAtomics) &&
	       (!a->shaderTessellationAndGeometryPointSize ||
		b->shaderTessellationAndGeometryPointSize) &&
	       (!a->shaderImageGatherExtended || b->shaderImageGatherExtended) &&
	       (!a->shaderStorageImageExtendedFormats || b->shaderStorageImageExtendedFormats) &&
	       (!a->shaderStorageImageMultisample || b->shaderStorageImageMultisample) &&
	       (!a->shaderStorageImageReadWithoutFormat ||
		b->shaderStorageImageReadWithoutFormat) &&
	       (!a->shaderStorageImageWriteWithoutFormat ||
		b->shaderStorageImageWriteWithoutFormat) &&
	       (!a->shaderUniformBufferArrayDynamicIndexing ||
		b->shaderUniformBufferArrayDynamicIndexing) &&
	       (!a->shaderSampledImageArrayDynamicIndexing ||
		b->shaderSampledImageArrayDynamicIndexing) &&
	       (!a->shaderStorageBufferArrayDynamicIndexing ||
		b->shaderStorageBufferArrayDynamicIndexing) &&
	       (!a->shaderStorageImageArrayDynamicIndexing ||
		b->shaderStorageImageArrayDynamicIndexing) &&
	       (!a->shaderClipDistance || b->shaderClipDistance) &&
	       (!a->shaderCullDistance || b->shaderCullDistance) &&
	       (!a->shaderFloat64 || b->shaderFloat64) &&
	       (!a->shaderInt64 || b->shaderInt64) &&
	       (!a->shaderInt16 || b->shaderInt16) &&
	       (!a->shaderResourceResidency || b->shaderResourceResidency) &&
	       (!a->shaderResourceMinLod || b->shaderResourceMinLod) &&
	       (!a->sparseBinding || b->sparseBinding) &&
	       (!a->sparseResidencyBuffer || b->sparseResidencyBuffer) &&
	       (!a->sparseResidencyImage2D || b->sparseResidencyImage2D) &&
	       (!a->sparseResidencyImage3D || b->sparseResidencyImage3D) &&
	       (!a->sparseResidency2Samples || b->sparseResidency2Samples) &&
	       (!a->sparseResidency4Samples || b->sparseResidency4Samples) &&
	       (!a->sparseResidency8Samples || b->sparseResidency8Samples) &&
	       (!a->sparseResidency16Samples || b->sparseResidency16Samples) &&
	       (!a->sparseResidencyAliased || b->sparseResidencyAliased) &&
	       (!a->variableMultisampleRate || b->variableMultisampleRate) &&
	       (!a->inheritedQueries || b->inheritedQueries);
}

bool DeviceQuery::queryDevice(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	uint32_t n;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, nullptr);
	vector<VkQueueFamilyProperties> queueFamilyProperties(n);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, queueFamilyProperties.data());

	VkQueueFlags foundQueues = 0;
	uint32_t i = 0;
	for (; i < n && (foundQueues & queues) != queues; i++)
	{
		VkQueueFlags flags = queueFamilyProperties[i].queueFlags;

		if (surface != VK_NULL_HANDLE && (flags & VK_QUEUE_GRAPHICS_BIT))
		{
			VkBool32 surfaceSupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &surfaceSupported);
			if (!surfaceSupported) flags &= ~VK_QUEUE_GRAPHICS_BIT;
		}

		foundQueues |= flags;
	}

	if ((foundQueues & queues) != queues) return false;

	if (features != nullptr)
	{
		VkPhysicalDeviceFeatures foundFeatures;
		vkGetPhysicalDeviceFeatures(device, &foundFeatures);
		if (!deviceFeatureIncludes(features, &foundFeatures)) return false;
	}

	if (!extensions.empty())
	{
		vkEnumerateDeviceExtensionProperties(device, nullptr, &n, nullptr);
		vector<VkExtensionProperties> extensionProperties(n);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &n,
						     extensionProperties.data());

		bool missing = false;
		for (i = 0; i < extensions.size() && !missing; i++)
		{
			uint32_t j = 0;
			for (; j < n && strcmp(extensions[i],
					       extensionProperties[j].extensionName); j++);
			missing = j == n;
		}
		if (missing) return false;
	}

	return true;
}

std::vector<VkPhysicalDevice>& DeviceQuery::getResults()
{
	if (results.empty())
	{
		for (uint32_t i = 0; i < physicalDeviceCount; i++)
		{
			if (queryDevice(physicalDevices[i]))
				results.push_back(physicalDevices[i]);
		}
	}
	return results;
}

}