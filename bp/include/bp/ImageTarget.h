#ifndef BP_IMAGETARGET_H
#define BP_IMAGETARGET_H

#include "Device.h"
#include "Pointer.h"
#include "RenderTarget.h"
#include "Image.h"
#include "FlagSet.h"

namespace bp
{

class ImageTarget : public RenderTarget
{
public:
	ImageTarget() :
		RenderTarget{},
		stagingImage{nullptr} {}
	ImageTarget(NotNull<Device> device, uint32_t width, uint32_t height,
		    const FlagSet<Flags>& flags =
		    	FlagSet<Flags>() << Flags::DEPTH_IMAGE << Flags::SHADER_READABLE) :
		ImageTarget{}
	{
		init(device, width, height, flags);
	}
	~ImageTarget() final;

	void init(NotNull<Device> device, uint32_t width, uint32_t height,
		  const FlagSet<Flags>& flags =
		  FlagSet<Flags>() << Flags::DEPTH_IMAGE << Flags::SHADER_READABLE);

	void beginFrame(VkCommandBuffer cmdBuffer) override;
	void endFrame(VkCommandBuffer cmdBuffer) override;
	void present(VkSemaphore renderCompleteSemaphore) override;
	void resize(uint32_t width, uint32_t height) override;

	Image* getImage() { return image; }
	Image* getStagingImage() { return stagingImage; }

private:
	Image* image;
	Image* stagingImage;

	void createImage();
	void createStagingImage();
};

}

#endif