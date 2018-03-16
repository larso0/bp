#ifndef BP_SORTLASTCOMPOSITOR_H
#define BP_SORTLASTCOMPOSITOR_H

#include "Compositor.h"

namespace bpMulti
{

class SortLastCompositor : public Compositor
{
public:

protected:
	void hostCopyStep() override
	{

	}
	void hostToDeviceStep(VkCommandBuffer cmdBuffer) override
	{

	}
};

}

#endif