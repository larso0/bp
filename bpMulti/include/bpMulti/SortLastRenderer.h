#ifndef BP_SORTLASTRENDERER_H
#define BP_SORTLASTRENDERER_H

#include "Renderer.h"

namespace bpMulti
{

class SortLastRenderer : public Renderer
{
public:
	virtual ~SortLastRenderer() = default;

	virtual void increaseWorkload(float hint) = 0;
	virtual void decreaseWorkload(float hint) = 0;
};

}

#endif