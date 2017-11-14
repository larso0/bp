#include <bp/DescriptorSet.h>
#include <stdexcept>
#include <bp/ImageDescriptor.h>
#include <bp/BufferDescriptor.h>

using namespace std;

namespace bp
{

void DescriptorSet::init(NotNull<Device> device, NotNull<DescriptorPool> pool,
			 NotNull<DescriptorSetLayout> layout)
{
	if (isReady()) throw runtime_error("Descriptor set already initialized.");
	this->device = device;
	this->pool = pool;

	VkDescriptorSetLayout layoutHandle = *layout;

	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = *pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &layoutHandle;

	VkResult result = vkAllocateDescriptorSets(*device, &info, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate descriptor set.");
}

DescriptorSet::~DescriptorSet()
{
	vkFreeDescriptorSets(*device, *pool, 1, &handle);
}

void DescriptorSet::bind(const NotNull<Descriptor> descriptor)
{
	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = handle;
	write.dstBinding = descriptor->getBinding();
	write.dstArrayElement = descriptor->getFirstIndex();
	write.descriptorType = descriptor->getType();

	{
		const ImageDescriptor* imageDescriptor =
			dynamic_cast<const ImageDescriptor*>(descriptor.get());
		if (imageDescriptor != nullptr)
		{
			const auto& infos = imageDescriptor->getDescriptorInfos();
			write.descriptorCount = static_cast<uint32_t>(infos.size());
			write.pImageInfo = infos.data();
		}
	}

	{
		const BufferDescriptor* bufferDescriptor =
			dynamic_cast<const BufferDescriptor*>(descriptor.get());
		if (bufferDescriptor != nullptr)
		{
			const auto& infos = bufferDescriptor->getDescriptorInfos();
			write.descriptorCount = static_cast<uint32_t>(infos.size());
			write.pBufferInfo = infos.data();
		}
	}

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