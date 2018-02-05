#ifndef BP_MEMORY_H
#define BP_MEMORY_H

namespace bp
{

class Memory
{
public:
	virtual ~Memory() = default;

	virtual bool isMapped() const { return false; }
	virtual void* getMapped() { return nullptr; }
	virtual void flushMapped() {}
};

}

#endif