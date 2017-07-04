#ifndef BP_H
#define BP_H

#include "vk_includes.h"
#include <vector>

namespace bp {
	extern std::vector<VkPhysicalDevice> physical_devices;

	void init();
}

#endif
