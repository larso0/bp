#ifndef BP_SORTFIRSTRENDERER_H
#define BP_SORTFIRSTRENDERER_H

#include <bp/Renderer.h>
#include <bpScene/Math.h>

namespace bpMulti
{

class SortFirstRenderer : public bp::Renderer
{
public:
	struct Contribution
	{
		glm::vec2 offset;
		glm::vec2 extent;
	};

	SortFirstRenderer() :
		contribution{{0.f, 0.f}, {1.f, 1.f}} {}
	virtual ~SortFirstRenderer() = default;

	void setContribution(const glm::vec2& offset, const glm::vec2 extent);

	const Contribution& getContribution() const { return contribution; }
	const glm::mat4& getContributionClipTransform() const { return contributionClipTransform; }

private:
	Contribution contribution;
	glm::mat4 contributionClipTransform;
};

}

#endif