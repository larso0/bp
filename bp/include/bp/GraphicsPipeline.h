#ifndef BP_GRAPHICSPIPELINE_H
#define BP_GRAPHICSPIPELINE_H

#include "Pipeline.h"

namespace bp
{

class GraphicsPipeline : public Pipeline
{
public:
	GraphicsPipeline() :
		Pipeline{},
		primitiveTopology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
		polygonMode{VK_POLYGON_MODE_FILL},
		cullMode{VK_CULL_MODE_BACK_BIT},
		frontFace{VK_FRONT_FACE_COUNTER_CLOCKWISE} {}
	GraphicsPipeline(NotNull<Device> device, NotNull<RenderPass> renderPass,
			 VkPipelineLayout layout,
			 std::initializer_list<VkPipelineShaderStageCreateInfo> shaderStageInfos,
			 std::initializer_list<VkVertexInputBindingDescription> bindingDescriptions,
			 std::initializer_list<VkVertexInputAttributeDescription> attributes,
			 VkPrimitiveTopology topology, VkPolygonMode polygonMode,
			 VkCullModeFlags cullMode, VkFrontFace frontFace) :
		Pipeline{},
		primitiveTopology{topology},
		polygonMode{polygonMode},
		cullMode{cullMode},
		frontFace{frontFace}
	{
		addShaderStageInfos(shaderStageInfos.begin(), shaderStageInfos.end());
		addVertexBindingDescriptions(bindingDescriptions.begin(),
					     bindingDescriptions.end());
		addVertexAttributeDescriptions(attributes.begin(), attributes.end());
		init(device, renderPass, layout);
	}
	
	void addVertexBindingDescription(const VkVertexInputBindingDescription& description)
	{
		vertexBindingDescriptions.push_back(description);
	}
	template <typename Iterator>
	void addVertexBindingDescriptions(Iterator begin, Iterator end)
	{
		vertexBindingDescriptions.insert(vertexBindingDescriptions.end(), begin, end);
	}
	void addVertexAttributeDescription(const VkVertexInputAttributeDescription& description)
	{
		vertexAttributeDescriptions.push_back(description);
	}
	template <typename Iterator>
	void addVertexAttributeDescriptions(Iterator begin, Iterator end)
	{
		vertexAttributeDescriptions.insert(vertexAttributeDescriptions.end(), begin, end);
	}

	void setPrimitiveTopology(VkPrimitiveTopology primitiveTopology)
	{
		GraphicsPipeline::primitiveTopology = primitiveTopology;
	}
	void setPolygonMode(VkPolygonMode polygonMode)
	{
		GraphicsPipeline::polygonMode = polygonMode;
	}
	void setCullMode(VkCullModeFlags cullMode)
	{
		GraphicsPipeline::cullMode = cullMode;
	}
	void setFrontFace(VkFrontFace frontFace)
	{
		GraphicsPipeline::frontFace = frontFace;
	}

	VkPrimitiveTopology getPrimitiveTopology() const
	{
		return primitiveTopology;
	}
	VkPolygonMode getPolygonMode() const
	{
		return polygonMode;
	}
	VkCullModeFlags getCullMode() const
	{
		return cullMode;
	}
	VkFrontFace getFrontFace() const
	{
		return frontFace;
	}

private:
	std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
	VkPrimitiveTopology primitiveTopology;
	VkPolygonMode polygonMode;
	VkCullModeFlags cullMode;
	VkFrontFace frontFace;
	
	void create() override;
};

}

#endif