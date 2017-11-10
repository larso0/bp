#include <bp/Compositor.h>
#include <bp/Util.h>
#include <stdexcept>
#include <future>
#include <GLFW/glfw3.h>

using namespace std;

namespace bp
{

Compositor::Compositor(RenderTarget& target) :
	renderPass{target, {{0, 0}, {target.getWidth(), target.getHeight()}}},
	renderer{nullptr},
	running{false},
	waitSem{VK_NULL_HANDLE}
{
}

Compositor::~Compositor()
{
	if (running) stop();
	if (renderer != nullptr) delete renderer;
	for (auto src : sources) delete src;
}

void Compositor::composite()
{
	if (renderer == nullptr)
	{
		renderer = new CompositeRenderer(renderPass, areas);
	}

	for (uint32_t i = 0; i < sources.size(); i++)
	{
		CompositeJob job = queue.dequeue();
		const auto& src = sources[job.jobIndex];
		VkDeviceSize size = src->target->getWidth() * src->target->getHeight() * 4;
		Image& image = renderer->getSourceImage(job.jobIndex);
		void* mapped = image.map(0, size);
		parallelCopy(mapped, job.imageData, size);
		image.unmap();
		job.compositeSem.signal();
	}
	nextFrameSem.signal(sources.size());

	renderer->render(waitSem);
	if (waitSem == VK_NULL_HANDLE) waitSem = renderPass.getRenderTarget().getPresentSemaphore();
	renderPass.getRenderTarget().present(renderer->getRenderCompleteSemaphore());
}

void Compositor::renderThread(uint32_t jobIndex, atomic<bool>& running,
				     AsyncQueue<CompositeJob>& queue, ImageTarget& target,
				     Renderer& renderer, CPUSemaphore& frameSem)
{
	CPUSemaphore compositeSem;
	renderer.render();

	double seconds = glfwGetTime();
	while (running)
	{
		target.present(renderer.getRenderCompleteSemaphore());

		future<void> renderFuture = async(launch::async, [&]
		{
			double time = glfwGetTime();
			float delta = (float) (time - seconds);
			seconds = time;
			renderer.update(delta);
			renderer.render(target.getPresentSemaphore());
		});

		frameSem.wait();
		const void* mapped = target.getStagingImage()->map(0, VK_WHOLE_SIZE);
		queue.enqueue({jobIndex, mapped, compositeSem});

		renderFuture.wait();

		compositeSem.wait();
		target.getStagingImage()->unmap(false);
	}
}

void Compositor::start()
{
	nextFrameSem.signal(sources.size());
	running = true;
	uint32_t i = 0;
	for (auto src : sources)
	{
		threads.emplace_back([&]{
			renderThread(i++, running, queue, *src->target, *src->renderer,
				     nextFrameSem);
		});
	}
}

void Compositor::stop()
{
	running = false;
	composite();
	for (auto& thread : threads) thread.join();
	threads.clear();
	waitSem = VK_NULL_HANDLE;
}

}