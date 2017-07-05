#include <bp/device.h>
#include <vector>

using namespace std;

namespace bp {
	extern VkInstance instance;
	extern vector<VkPhysicalDevice> physical_devices;

	void device::capabilities(capability c) {
		m_capabilities = c;
	}

	void device::features(feature f) {
		m_features = f;
	}
}

BP_DEFINE_BITMASK_OPERATORS(bp::device::capability)

BP_DEFINE_BITMASK_OPERATORS(bp::device::feature)