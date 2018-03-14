#ifndef BP_PIPELINESTEP_H
#define BP_PIPELINESTEP_H

#include <vector>

namespace bpMulti
{

template<typename Output, typename... Input>
class PipelineStep
{
public:
	virtual ~PipelineStep()
	{
		for (auto& output : outputs) destroy(output);
	}

	void init(unsigned outputCount)
	{
		outputs.resize(outputCount);
		for (unsigned i = 0; i < outputCount; i++) prepare(outputs[i]);
	}

	Output& process(Input... input, unsigned outputIdx)
	{
		process(input..., outputs[outputIdx], outputIdx);
		return outputs[outputIdx];
	}

	unsigned getOutputCount() const { return static_cast<unsigned>(outputs.size()); }
	Output& getOutput(unsigned index) { return outputs[index]; }

protected:
	virtual void prepare(Output&) {}
	virtual void destroy(Output&) {}
	virtual void process(Input... input, Output& output, unsigned outputIndx) = 0;

private:
	std::vector<Output> outputs;
};

}

#endif