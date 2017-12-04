#ifndef BP_RENDERTARGET_H
#define BP_RENDERTARGET_H

#include "Device.h"
#include "Pointer.h"
#include "Image.h"
#include "Buffer.h"
#include "FlagSet.h"
#include <vector>

namespace bp
{

class RenderTarget
{
public:
	enum class Flags : size_t
	{
		DEPTH_IMAGE,
		DEPTH_STAGING_BUFFER,
		STAGING_IMAGE,
		SHADER_READABLE,
		VERTICAL_SYNC,
		BP_FLAGSET_LAST
	};

	RenderTarget() :
		device{nullptr},
		format{VK_FORMAT_UNDEFINED},
		width{0}, height{0},
		cmdPool{VK_NULL_HANDLE},
		depthImage{nullptr},
		depthImageView{VK_NULL_HANDLE},
		depthStagingBuffer{nullptr},
		framebufferImageCount{0},
		currentFramebufferIndex{0},
		presentSemaphore{VK_NULL_HANDLE} {}
	RenderTarget(NotNull<Device> device, VkFormat format, uint32_t width, uint32_t height,
		     FlagSet<Flags> flags) :
		RenderTarget{}
	{
		init(device, format, width, height, flags);
	}
	virtual ~RenderTarget();

	void init(NotNull<Device> device, VkFormat format, uint32_t width, uint32_t height,
		  FlagSet<Flags> flags);

	virtual void beginFrame(VkCommandBuffer cmdBuffer) = 0;
	virtual void endFrame(VkCommandBuffer cmdBuffer) = 0;
	virtual void present(VkSemaphore renderCompleteSemaphore) = 0;
	virtual void resize(uint32_t width, uint32_t height);

	Device* getDevice() { return device; }
	VkCommandPool getCmdPool() { return cmdPool; }
	bool isDepthImageEnabled() const { return flags & Flags::DEPTH_IMAGE; }
	Image* getDepthImage() { return depthImage; }
	Buffer* getDepthStagingBuffer() { return depthStagingBuffer; }
	VkImageView getDepthImageView() { return depthImageView; }
	VkFormat getFormat() const { return format; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	uint32_t getFramebufferImageCount() const { return framebufferImageCount; }
	std::vector<VkImageView>& getFramebufferImageViews() { return framebufferImageViews; };
	uint32_t getCurrentFramebufferIndex() const { return currentFramebufferIndex; }
	VkSemaphore getPresentSemaphore() { return presentSemaphore; }
	bool isReady() const { return cmdPool != VK_NULL_HANDLE; }

protected:
	FlagSet<Flags> flags;
	Device* device;
	VkFormat format;
	uint32_t width, height;

	VkCommandPool cmdPool;
	Image* depthImage;
	VkImageView depthImageView;
	Buffer* depthStagingBuffer;

	uint32_t framebufferImageCount;
	std::vector<VkImageView> framebufferImageViews;
	uint32_t currentFramebufferIndex;
	VkSemaphore presentSemaphore;

	void createDepthImage();
	void createDepthStagingBuffer();
	void assertReady();
};

}

#endif