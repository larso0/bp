#ifndef BP_MEMORY_H
#define BP_MEMORY_H

#include <vulkan/vulkan.h>

namespace bp
{

class Memory
{
public:
	virtual ~Memory() = default;

	virtual bool isMapped() const { return false; }
	virtual VkDeviceSize getSize() const = 0;
	virtual void* getMapped() { return nullptr; }
	virtual void flushMapped() {}
};

}

#endif