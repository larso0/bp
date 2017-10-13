#ifndef BP_IMAGETARGET_H
#define BP_IMAGETARGET_H

#include "RenderTarget.h"
#include "Image.h"
#include "FlagSet.h"

namespace bp
{

class ImageTarget : public RenderTarget
{
public:
	enum class Flags : size_t
	{
		DEPTH_IMAGE,
		STAGING_IMAGE,
		SHADER_READABLE,
		BP_FLAGSET_LAST
	};

	ImageTarget(Device& device, uint32_t width, uint32_t height,
		    const FlagSet<Flags>& flags =
		    	FlagSet<Flags>() << Flags::DEPTH_IMAGE << Flags::SHADER_READABLE);

	~ImageTarget() final;

	void beginFrame(VkCommandBuffer cmdBuffer) override;
	void endFrame(VkCommandBuffer cmdBuffer) override;
	void present(VkSemaphore renderCompleteSemaphore) override;
	void resize(uint32_t width, uint32_t height) override;

	Image* getStagingImage() { return stagingImage; }

private:
	FlagSet<Flags> flags;
	Image* image;
	Image* stagingImage;

	void createImage();
	void createStagingImage();
};

}

#endif