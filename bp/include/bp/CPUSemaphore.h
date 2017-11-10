#ifndef BP_CPUSEMAPHORE_H
#define BP_CPUSEMAPHORE_H

#include <mutex>
#include <condition_variable>

namespace bp
{

class CPUSemaphore
{
public:
	explicit CPUSemaphore(unsigned v = 0) :
		value{v} {}

	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		while (value == 0) conditionVariable.wait(lock);
		value--;
	}

	void signal(unsigned n = 1)
	{
		std::unique_lock<std::mutex> lock(mutex);
		value += n;
		for (unsigned i = 0; i < n; i++) conditionVariable.notify_one();
	}

private:
	std::mutex mutex;
	std::condition_variable conditionVariable;
	unsigned value;
};

}

#endif