#ifndef BP_IMAGETARGET_H
#define BP_IMAGETARGET_H

#include "RenderTarget.h"
#include "Image.h"

namespace bp
{

class ImageTarget : public RenderTarget
{
public:
	ImageTarget() :
		RenderTarget(),
		image(nullptr),
		stagingImage(nullptr)
	{
		format = VK_FORMAT_R8G8B8A8_UNORM;
		imageCount = 1;
	}
	~ImageTarget() final;

	void init() override;
	void beginFrame(VkCommandBuffer cmdBuffer) override;
	void endFrame(VkCommandBuffer cmdBuffer) override;
	void present(VkSemaphore renderCompleteSemaphore) override;
	void resize(uint32_t width, uint32_t height) override;

	Image* getStagingImage() { return stagingImage; }
	bool isReady() const override { return RenderTarget::isReady() && image != nullptr; }

private:
	Image* image;
	Image* stagingImage;

	void createImage();
	void createStagingImage();
};

}

#endif