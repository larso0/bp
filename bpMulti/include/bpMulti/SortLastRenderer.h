#ifndef BP_SORTLASTRENDERER_H
#define BP_SORTLASTRENDERER_H

#include <bp/Renderer.h>

namespace bpMulti
{

class SortLastRenderer : public bp::Renderer
{
public:
	virtual ~SortLastRenderer() = default;

	virtual void increaseWorkload(float hint) = 0;
	virtual void decreaseWorkload(float hint) = 0;
};

}

#endif