#include <bp/DeviceRequirements.h>
#include <cstring>

using namespace std;

namespace bp
{

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

bool queryDevice(VkPhysicalDevice device, const DeviceRequirements& requirements)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	// Check that the required features are available
	VkPhysicalDeviceFeatures foundFeatures;
	vkGetPhysicalDeviceFeatures(device, &foundFeatures);
	if (!deviceFeatureIncludes(&requirements.features, &foundFeatures)) return false;

	uint32_t n;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, nullptr);
	vector<VkQueueFamilyProperties> queueFamilyProperties(n);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, queueFamilyProperties.data());

	// Check required queue capabilities
	VkQueueFlags foundQueues = 0;
	for (auto i; i < n && requirements.meetsQueueCapabilities(foundQueues); i++)
	{
		VkQueueFlags flags = queueFamilyProperties[i].queueFlags;

		if (requirements.surface != VK_NULL_HANDLE && (flags & VK_QUEUE_GRAPHICS_BIT))
		{
			VkBool32 surfaceSupported;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, requirements.surface,
							     &surfaceSupported);
			if (!surfaceSupported) flags &= ~VK_QUEUE_GRAPHICS_BIT;
		}

		foundQueues |= flags;
	}

	if (requirements.meetsQueueCapabilities(foundQueues)) return false;

	// Check for exact queue families and queue count
	if (!requirements.queues.empty())
	{
		vector<bool> resolved(requirements.queues.size(), 0);

		for (auto i = 0; i < n; i++)
		{
			VkQueueFlags flags = queueFamilyProperties[i].queueFlags;
			uint32_t count = queueFamilyProperties[i].queueCount;

			for (auto j = 0; j < resolved.size(); j++)
			{
				QueueRequirements required = requirements.queues[j];
				if ((flags & required.flags) == required.flags
				    && count >= required.count)
				{
					if (required.surface != VK_NULL_HANDLE)
					{
						VkBool32 surfaceSupported;
						vkGetPhysicalDeviceSurfaceSupportKHR(
							device, i, required.surface,
							&surfaceSupported);
						if (surfaceSupported) resolved[j] = true;
					} else
					{
						resolved[j] = true;
					}
				}
			}
		}

		for (bool found : resolved) if (!found) return false;
	}

	// Check required extensions
	if (!requirements.extensions.empty())
	{
		vkEnumerateDeviceExtensionProperties(device, nullptr, &n, nullptr);
		vector<VkExtensionProperties> extensionProperties(n);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &n,
						     extensionProperties.data());

		bool missing = false;
		for (auto i = 0; i < requirements.extensions.size() && !missing; i++)
		{
			uint32_t j = 0;
			for (; j < n && strcmp(requirements.extensions[i].c_str(),
					       extensionProperties[j].extensionName); j++);
			missing = j == n;
		}
		if (missing) return false;
	}

	// Check required layers
	if (!requirements.layers.empty())
	{
		vkEnumerateDeviceLayerProperties(device, &n, nullptr);
		vector<VkLayerProperties> layerProperties(n);
		vkEnumerateDeviceLayerProperties(device, &n, layerProperties.data());

		bool missing = false;
		for (auto i = 0; i < requirements.layers.size() && !missing; i++)
		{
			uint32_t j = 0;
			for (; j < n && strcmp(requirements.layers[i].c_str(),
					       layerProperties[j].layerName); j++);
			missing = j == n;
		}
		if (missing) return false;
	}

	return true;
}

vector<VkPhysicalDevice> queryDevices(const vector<VkPhysicalDevice>& devices,
				      const DeviceRequirements& requirements)
{
	vector<VkPhysicalDevice> results;
	for (VkPhysicalDevice device : devices)
	{
		if (queryDevice(device, requirements))
			results.push_back(device);
	}
	return results;
}

vector<VkPhysicalDevice> queryDevices(const Instance& instance,
				      const DeviceRequirements& requirements)
{
	return queryDevices(instance.getPhysicalDevices(), requirements);
}

}