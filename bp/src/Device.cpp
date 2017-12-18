#include <bp/Device.h>
#include <bp/Util.h>
#include <stdexcept>
#include <cstring>

using namespace std;

namespace bp
{

void Device::init(const Instance& instance, const DeviceRequirements& requirements)
{
	if (isReady()) throw runtime_error("Device is already initialized.");
	auto result = queryDevices(instance, requirements);
	if (result.empty())
		throw runtime_error("No suitable physical device found.");
	physical = result[0];

	createLogicalDevice(requirements);
	createQueues();
}

void Device::init(VkPhysicalDevice physicalDevice, const DeviceRequirements& requirements)
{
	if (physicalDevice == VK_NULL_HANDLE)
		throw invalid_argument("Physical device must be a valid handle.");

	bool suitable = queryDevice(physicalDevice, requirements);
	if (!suitable)
		throw runtime_error("Given physical device is not suitable.");
	physical = physicalDevice;

	try
	{
		createLogicalDevice(requirements);
		createQueues();
	} catch (exception& e)
	{
		if (isReady())
		{
			queueInfos.clear();
			queues.clear();
			vkDestroyDevice(logical, nullptr);
		}
		physical = VK_NULL_HANDLE;
		logical = VK_NULL_HANDLE;
		throw e;
	}
}

Device::~Device()
{
	queues.clear();
	vkDestroyDevice(logical, nullptr);
}

Queue& Device::getQueue(uint32_t index)
{
	assertReady();
	if (index >= getQueueCount())
		throw out_of_range("Invalid queue index.");
	return queues[index];
}

Queue& Device::getGraphicsQueue()
{
	assertReady();
	for (auto i = 0; i < getQueueCount(); i++)
		if (queueInfos[i].flags & VK_QUEUE_GRAPHICS_BIT) return queues[i];
	throw runtime_error("No graphics queue available.");
}

Queue& Device::getComputeQueue()
{
	assertReady();
	for (auto i = 0; i < getQueueCount(); i++)
		if (queueInfos[i].flags & VK_QUEUE_COMPUTE_BIT) return queues[i];
	throw runtime_error("No compute queue available.");
}

Queue& Device::getTransferQueue()
{
	assertReady();
	for (auto i = 0; i < getQueueCount(); i++)
		if (queueInfos[i].flags & VK_QUEUE_TRANSFER_BIT) return queues[i];
	throw runtime_error("No transfer queue available.");
}

Queue& Device::getSparseBindingQueue()
{
	assertReady();
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

	vector<const char*> enabledLayers;
	enabledLayers.reserve(requirements.layers.size());
	for (auto l : requirements.layers)
		enabledLayers.push_back(l.c_str());

	info.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
	info.ppEnabledLayerNames = enabledLayers.data();

	vector<const char*> enabledExtensions;
	for (auto ext : requirements.extensions)
		enabledExtensions.push_back(ext.c_str());

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

	for (auto i = 0; i < getQueueCount(); i++)
	{
		if (queueInfos[i].flags & VK_QUEUE_GRAPHICS_BIT)
		{

		}
	}
}

vector<VkDeviceQueueCreateInfo>
Device::setupQueueCreateInfos(const DeviceRequirements& requirements)
{
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	static const float priority = 1.f;

	uint32_t n;
	vkGetPhysicalDeviceQueueFamilyProperties(physical, &n, NULL);
	vector<VkQueueFamilyProperties> queueFamilyProperties(n);
	vkGetPhysicalDeviceQueueFamilyProperties(physical, &n, queueFamilyProperties.data());

	auto updateCreateInfos = [&](uint32_t qfi, uint32_t count)
	{
		bool found = false;
		for (auto& createInfo : queueCreateInfos)
		{
			if (createInfo.queueFamilyIndex == qfi)
			{
				createInfo.queueCount += count;
				if (queueFamilyProperties[qfi].queueCount < createInfo.queueCount)
					throw runtime_error("Not enough queues available.");
			}
		}
	};

	auto findExistingCreateInfo = [&](VkQueueFlags flags) -> int
	{

	};

	for (const QueueRequirements& q : requirements.queues)
	{
		uint32_t qfi;
		bool foundQfi = findQueueFamilyIndex(physical, q.flags, &qfi, q.count, q.surface);
		if (!foundQfi)
		{
			throw runtime_error(
				"Specified queue requirements not met by any queue "
				"family indices.");
		}

		bool found = false;
		for (auto& createInfo : queueCreateInfos)
		{
			if (createInfo.queueFamilyIndex == qfi)
			{
				createInfo.queueCount += q.count;
				if (queueFamilyProperties[qfi].queueCount < createInfo.queueCount)
					throw runtime_error("Not enough queues available.");
			}
		}

		queueCreateInfos.push_back({VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0,
					    static_cast<uint32_t>(qfi), q.count,
					    q.priorities.data()});
		for (uint32_t i = 0; i < q.count; i++)
			queueInfos.push_back({static_cast<uint32_t>(qfi), i, q.flags});
	}

	if (requirements.queueCapabilities & VK_QUEUE_GRAPHICS_BIT)
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
			if (q.familyIndex == qfi)
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
			if (q.familyIndex == qfi)
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

	return move(queueCreateInfos);
}

void Device::assertReady()
{
	if (!isReady())
		throw runtime_error("Device not ready. Must initialize before use.");
}

}