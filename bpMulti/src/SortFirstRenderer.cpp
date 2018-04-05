#include <bpMulti/SortFirstRenderer.h>

namespace bpMulti
{

void SortFirstRenderer::setContribution(const glm::vec2& offset, const glm::vec2 extent)
{
	contribution = {offset, extent};
	contributionClipTransform =
		glm::scale(glm::translate(glm::mat4{},
					  {(2.f * offset + extent - 1.f) / extent, 0.f}),
			   {1.f / extent, 1.f});
	contributionChangedEvent();
}

}