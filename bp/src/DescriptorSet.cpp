#include <bp/DescriptorSet.h>
#include <stdexcept>
#include <bp/ImageDescriptor.h>
#include <bp/BufferDescriptor.h>

using namespace std;

namespace bp
{

void DescriptorSet::init(Device& device, DescriptorPool& pool,
			 DescriptorSetLayout& layout)
{
	if (isReady()) throw runtime_error("Descriptor set already initialized.");
	DescriptorSet::device = &device;
	DescriptorSet::pool = &pool;

	VkDescriptorSetLayout layoutHandle = layout;

	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &layoutHandle;

	VkResult result = vkAllocateDescriptorSets(device, &info, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate descriptor set.");
}

DescriptorSet::~DescriptorSet()
{
	vkFreeDescriptorSets(*device, *pool, 1, &handle);
}

void DescriptorSet::bind(const Descriptor& descriptor)
{
	VkWriteDescriptorSet write = descriptor.getWriteInfo();
	write.dstSet = handle;

	descriptorWrites.push_back(write);
}

void DescriptorSet::update()
{
	if (descriptorWrites.empty()) return;
	vkUpdateDescriptorSets(*device, static_cast<uint32_t>(descriptorWrites.size()),
			       descriptorWrites.data(), 0, nullptr);
	descriptorWrites.clear();
}

}