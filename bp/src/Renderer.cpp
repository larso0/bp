#include <bp/Renderer.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void Renderer::setRenderPass(RenderPass* renderPass)
{
	if (isReady())
		throw runtime_error("Failed to alter render pass, renderer already created.");
	this->renderPass = renderPass;
}

}