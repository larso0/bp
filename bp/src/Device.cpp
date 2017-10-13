#include <bp/Device.h>
#include <bp/Util.h>
#include <stdexcept>
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

	uint32_t n;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, nullptr);
	vector<VkQueueFamilyProperties> queueFamilyProperties(n);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &n, queueFamilyProperties.data());

	VkQueueFlags foundQueues = 0;
	uint32_t i = 0;
	for (; i < n && (foundQueues & requirements.queues) != requirements.queues; i++)
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

	if ((foundQueues & requirements.queues) != requirements.queues) return false;

	VkPhysicalDeviceFeatures foundFeatures;
	vkGetPhysicalDeviceFeatures(device, &foundFeatures);
	if (!deviceFeatureIncludes(&requirements.features, &foundFeatures)) return false;

	if (!requirements.extensions.empty())
	{
		vkEnumerateDeviceExtensionProperties(device, nullptr, &n, nullptr);
		vector<VkExtensionProperties> extensionProperties(n);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &n,
						     extensionProperties.data());

		bool missing = false;
		for (i = 0; i < requirements.extensions.size() && !missing; i++)
		{
			uint32_t j = 0;
			for (; j < n && strcmp(requirements.extensions[i],
					       extensionProperties[j].extensionName); j++);
			missing = j == n;
		}
		if (missing) return false;
	}

	return true;
}

vector<VkPhysicalDevice> queryDevices(const Context& context,
				      const DeviceRequirements& requirements)
{
	vector<VkPhysicalDevice> results;
	for (VkPhysicalDevice device : context.getPhysicalDevices())
	{
		if (queryDevice(device, requirements))
			results.push_back(device);
	}
	return results;
}

Device::Device(const Context& context, const DeviceRequirements& requirements)
{
	auto result = queryDevices(context, requirements);
	if (result.empty())
		throw runtime_error("No suitable physical device found.");
	physical = result[0];

	createLogicalDevice(requirements);
	createQueues();
}

Device::Device(VkPhysicalDevice physicalDevice, const DeviceRequirements& requirements)
{
	bool suitable = queryDevice(physicalDevice, requirements);
	if (!suitable)
		throw runtime_error("Given physical device is not suitable.");
	physical = physicalDevice;

	createLogicalDevice(requirements);
	createQueues();
}

Device::~Device()
{
	queues.clear();
	vkDestroyDevice(logical, nullptr);
}

Queue& Device::getQueue(uint32_t index)
{
	if (index >= getQueueCount())
		throw out_of_range("Invalid queue index.");
	return queues[index];
}

Queue& Device::getGraphicsQueue()
{
	for (auto i = 0; i < getQueueCount(); i++)
		if (queueInfos[i].flags & VK_QUEUE_GRAPHICS_BIT) return queues[i];
	throw runtime_error("No graphics queue available.");
}

Queue& Device::getComputeQueue()
{
	for (auto i = 0; i < getQueueCount(); i++)
		if (queueInfos[i].flags & VK_QUEUE_COMPUTE_BIT) return queues[i];
	throw runtime_error("No compute queue available.");
}

Queue& Device::getTransferQueue()
{
	for (auto i = 0; i < getQueueCount(); i++)
		if (queueInfos[i].flags & VK_QUEUE_TRANSFER_BIT) return queues[i];
	throw runtime_error("No transfer queue available.");
}

Queue& Device::getSparseBindingQueue()
{
	for (auto i = 0; i < getQueueCount(); i++)
		if (queueInfos[i].flags & VK_QUEUE_SPARSE_BINDING_BIT) return queues[i];
	throw runtime_error("No sparse binding queue available.");
}

void Device::createLogicalDevice(const DeviceRequirements& requirements)
{
	auto queueCreateInfos = setupQueueCreateInfos(requirements);

	VkDeviceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	info.pQueueCreateInfos = queueCreateInfos.data();

	vector<const char*> enabledExtensions;
	for (auto ext : requirements.extensions)
		enabledExtensions.push_back(ext);

	info.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	info.ppEnabledExtensionNames = enabledExtensions.data();
	info.pEnabledFeatures = &requirements.features;

	VkResult result = vkCreateDevice(physical, &info, nullptr, &logical);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create logical device.");
}

void Device::createQueues()
{
	for (auto& q : queueInfos)
		queues.emplace_back(logical, q.familyIndex, 0);
}

vector<VkDeviceQueueCreateInfo>
Device::setupQueueCreateInfos(const DeviceRequirements& requirements)
{
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	static const float priority = 1.f;

	if (requirements.queues & VK_QUEUE_GRAPHICS_BIT)
	{
		int32_t qfi = -1;
		if (requirements.surface != VK_NULL_HANDLE)
			qfi = findSurfaceQueueFamilyIndex(physical, requirements.surface);
		else
			qfi = findQueueFamilyIndex(physical, VK_QUEUE_GRAPHICS_BIT);
		if (qfi == -1)
			throw runtime_error("No graphics queue family available.");
		queueCreateInfos.push_back({VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
					    static_cast<uint32_t>(qfi), 1, &priority});
		queueInfos.push_back({static_cast<uint32_t>(qfi), VK_QUEUE_GRAPHICS_BIT});
	}

	if (requirements.queues & VK_QUEUE_COMPUTE_BIT)
	{
		int32_t qfi = findQueueFamilyIndex(physical, VK_QUEUE_COMPUTE_BIT);
		if (qfi == -1)
			throw runtime_error("No compute queue family available.");

		bool found = false;
		for (auto& q : queueInfos)
		{
			if (q.familyIndex)
			{
				q.flags |= VK_QUEUE_COMPUTE_BIT;
				found = true;
				break;
			}
		}

		if (!found)
		{
			queueCreateInfos.push_back(
				{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
				 static_cast<uint32_t>(qfi), 1, &priority});
			queueInfos.push_back({static_cast<uint32_t>(qfi), VK_QUEUE_COMPUTE_BIT});
		}
	}

	if (requirements.queues & VK_QUEUE_TRANSFER_BIT)
	{
		int32_t qfi = findQueueFamilyIndex(physical, VK_QUEUE_TRANSFER_BIT);
		if (qfi == -1)
			throw runtime_error("No transfer queue family available.");

		bool found = false;
		for (auto& q : queueInfos)
		{
			if (q.familyIndex)
			{
				q.flags |= VK_QUEUE_TRANSFER_BIT;
				found = true;
				break;
			}
		}

		if (!found)
		{
			queueCreateInfos.push_back(
				{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
				 static_cast<uint32_t>(qfi), 1, &priority});
			queueInfos.push_back({static_cast<uint32_t>(qfi), VK_QUEUE_TRANSFER_BIT});
		}
	}

	if (requirements.queues & VK_QUEUE_SPARSE_BINDING_BIT)
	{
		int32_t qfi = findQueueFamilyIndex(physical, VK_QUEUE_SPARSE_BINDING_BIT);
		if (qfi == -1)
			throw runtime_error("No sparse binding queue family available.");

		bool found = false;
		for (auto& q : queueInfos)
		{
			if (q.familyIndex)
			{
				q.flags |= VK_QUEUE_SPARSE_BINDING_BIT;
				found = true;
				break;
			}
		}

		if (!found)
		{
			queueCreateInfos.push_back(
				{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
				 static_cast<uint32_t>(qfi), 1, &priority});
			queueInfos.push_back(
				{static_cast<uint32_t>(qfi), VK_QUEUE_SPARSE_BINDING_BIT});
		}
	}

	return queueCreateInfos;
}

}