#ifndef BP_BUFFERDESCRIPTOR_H
#define BP_BUFFERDESCRIPTOR_H

#include "Descriptor.h"
#include <vector>

namespace bp
{

class BufferDescriptor : public Descriptor
{
public:
	BufferDescriptor() :
		Descriptor{} {}
	BufferDescriptor(VkDescriptorType type, uint32_t binding, uint32_t firstIndex,
			const std::vector<VkDescriptorBufferInfo>& infos) :
		Descriptor{type, binding, firstIndex},
		descriptorInfos{infos} {}
	virtual ~BufferDescriptor() = default;

	void addDescriptorInfo(const VkDescriptorBufferInfo& info)
	{
		descriptorInfos.push_back(info);
	}

	const std::vector<VkDescriptorBufferInfo>& getDescriptorInfos() const
	{
		return descriptorInfos;
	}

private:
	std::vector<VkDescriptorBufferInfo> descriptorInfos;
};

}

#endif