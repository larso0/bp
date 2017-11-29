#ifndef BP_PIPELINELAYOUT_H
#define BP_PIPELINELAYOUT_H

#include "Device.h"
#include "Pointer.h"
#include <vector>
#include <initializer_list>

namespace bp
{

class PipelineLayout
{
public:
	PipelineLayout() :
		device{nullptr},
		handle{VK_NULL_HANDLE} {}
	PipelineLayout(NotNull<Device> device,
		       std::initializer_list<VkDescriptorSetLayout> setLayouts,
		       std::initializer_list<VkPushConstantRange> pushConstantRanges) :
		PipelineLayout{}
	{
		addDescriptorSetLayouts(setLayouts.begin(), setLayouts.end());
		addPushConstantRanges(pushConstantRanges.begin(), pushConstantRanges.end());
		init(device);
	}
	~PipelineLayout();

	void addDescriptorSetLayout(VkDescriptorSetLayout setLayout)
	{
		setLayouts.push_back(setLayout);
	}

	template <typename Iterator>
	void addDescriptorSetLayouts(Iterator begin, Iterator end)
	{
		setLayouts.insert(setLayouts.end(), begin, end);
	}

	void addPushConstantRange(const VkPushConstantRange& pushConstantRange)
	{
		pushConstantRanges.push_back(pushConstantRange);
	}

	template <typename Iterator>
	void addPushConstantRanges(Iterator begin, Iterator end)
	{
		pushConstantRanges.insert(pushConstantRanges.end(), begin, end);
	}

	void init(NotNull<Device> device);
	void init(NotNull<Device> device,
		  std::initializer_list<VkDescriptorSetLayout> setLayouts,
		  std::initializer_list<VkPushConstantRange> pushConstantRanges)
	{
		addDescriptorSetLayouts(setLayouts.begin(), setLayouts.end());
		addPushConstantRanges(pushConstantRanges.begin(), pushConstantRanges.end());
		init(device);
	}

	operator VkPipelineLayout() { return handle; }

	bool isReady() const { return handle != VK_NULL_HANDLE; }
	VkPipelineLayout getHandle() { return handle; }

private:
	Device* device;
	VkPipelineLayout handle;
	std::vector<VkDescriptorSetLayout> setLayouts;
	std::vector<VkPushConstantRange> pushConstantRanges;
};

}

#endif