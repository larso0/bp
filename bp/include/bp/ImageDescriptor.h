#ifndef BP_IMAGEDESCRIPTOR_H
#define BP_IMAGEDESCRIPTOR_H

#include "Descriptor.h"
#include <vector>

namespace bp
{

class ImageDescriptor : public Descriptor
{
public:
	ImageDescriptor() :
		Descriptor{} {}
	ImageDescriptor(VkDescriptorType type, uint32_t binding, uint32_t firstIndex,
			const std::vector<VkDescriptorImageInfo>& infos) :
		Descriptor{type, binding, firstIndex},
		descriptorInfos{infos} {}
	virtual ~ImageDescriptor() = default;

	void resetDescriptorInfos() { descriptorInfos.clear(); }

	void addDescriptorInfo(const VkDescriptorImageInfo& info)
	{
		descriptorInfos.push_back(info);
	}

	const std::vector<VkDescriptorImageInfo>& getDescriptorInfos() const
	{
		return descriptorInfos;
	}

	VkWriteDescriptorSet getWriteInfo() const override
	{
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = getBinding();
		write.dstArrayElement = getFirstIndex();
		write.descriptorType = getType();
		write.descriptorCount = static_cast<uint32_t>(descriptorInfos.size());
		write.pImageInfo = descriptorInfos.data();
		return write;
	}

private:
	std::vector<VkDescriptorImageInfo> descriptorInfos;
};

}

#endif