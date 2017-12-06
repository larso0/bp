#ifndef BP_QT_DEVICEQUERY_H
#define BP_QT_DEVICEQUERY_H

#include <bp/Device.h>
#include <QVulkanInstance>

namespace bpQt
{

std::vector<VkPhysicalDevice> queryDevices(const QVulkanInstance& instance,
					   const bp::DeviceRequirements& requirements);

}

#endif