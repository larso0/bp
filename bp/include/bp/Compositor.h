#ifndef BP_COMPOSITOR_H
#define BP_COMPOSITOR_H

#include "CompositeRenderer.h"
#include "RenderPass.h"
#include "ImageTarget.h"
#include "AsyncQueue.h"
#include "CPUSemaphore.h"
#include <vector>
#include <mutex>
#include <utility>
#include <atomic>
#include <thread>

namespace bp
{

class Compositor
{
public:
	explicit Compositor(RenderTarget& target);
	~Compositor();

	void composite(bool waitForPresent = true);
	void start();
	void stop();

	template <class DerivedRenderer, typename... ParamTypes>
	std::pair<DerivedRenderer*, std::mutex*> addSource(Device& device, const VkRect2D& area,
						    const VkClearValue& clearValue,
						    ParamTypes... constructArgs)
	{
		static_assert(std::is_base_of<Renderer, DerivedRenderer>::value,
			      "DerivedRenderer must derive bp::Renderer");
		ImageTarget* target = new ImageTarget(device, area.extent.width, area.extent.height,
						      FlagSet<ImageTarget::Flags>()
							      << ImageTarget::Flags::STAGING_IMAGE
							      << ImageTarget::Flags::DEPTH_IMAGE);
		RenderPass* pass = new RenderPass(*target, {{0, 0}, area.extent}, clearValue);
		DerivedRenderer* renderer = new DerivedRenderer(*pass, constructArgs...);
		uint32_t index = static_cast<uint32_t>(sources.size());
		sources.emplace_back(new CompositeSource(target, pass, static_cast<Renderer*>(renderer), index));
		areas.push_back(area);
		return {renderer, sources[index]->mutex};
	}

private:
	struct CompositeSource
	{
		CompositeSource(ImageTarget* target, RenderPass* pass, Renderer* renderer,
				uint32_t index) :
			mutex{new std::mutex()},
			target{target},
			renderPass{renderPass},
			renderer{renderer},
			index{index} {}
		~CompositeSource()
		{
			delete renderer;
			delete renderPass;
			delete target;
			delete mutex;
		}

		std::mutex* mutex;
		ImageTarget* target;
		RenderPass* renderPass;
		Renderer* renderer;
		uint32_t index;
	};

	struct CompositeJob
	{
		uint32_t jobIndex;
		const void* imageData;
		CPUSemaphore& compositeSem;
	};

	std::vector<CompositeSource*> sources;
	std::vector<VkRect2D> areas;
	RenderPass renderPass;
	CompositeRenderer* renderer;
	std::atomic<bool> running;
	AsyncQueue<CompositeJob> queue;
	std::vector<std::thread> threads;
	VkSemaphore waitSem;
	CPUSemaphore nextFrameSem;

	static void renderThread(uint32_t jobIndex, std::atomic<bool>& running,
				 AsyncQueue<Compositor::CompositeJob>& queue, ImageTarget& target,
				 Renderer& renderer, CPUSemaphore& frameSem);
};

}

#endif