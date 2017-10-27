#ifndef BP_IMAGEDESCRIPTOR_H
#define BP_IMAGEDESCRIPTOR_H

#include "Descriptor.h"
#include <vector>

namespace bp
{

class ImageDescriptor : public Descriptor
{
public:
	ImageDescriptor(VkDescriptorType type, uint32_t binding, uint32_t firstIndex,
			const std::vector<VkDescriptorImageInfo>& infos) :
		Descriptor{type, binding, firstIndex},
		descriptorInfos{infos} {}
	virtual ~ImageDescriptor() = default;

	const std::vector<VkDescriptorImageInfo>& getDescriptorInfos() const
	{
		return descriptorInfos;
	}

private:
	std::vector<VkDescriptorImageInfo> descriptorInfos;
};

}

#endif