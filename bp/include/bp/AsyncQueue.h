#ifndef BP_ASYNCQUEUE_H
#define BP_ASYNCQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

namespace bp
{

template<typename T>
class AsyncQueue
{
public:
	void enqueue(T element)
	{
		std::lock_guard<std::mutex> lock(mutex);
		queue.push(element);
		enqueueNotifier.notify_one();
	}

	T dequeue()
	{
		std::unique_lock<std::mutex> lock(mutex);
		while (queue.empty()) enqueueNotifier.wait(lock);
		T element = queue.front();
		queue.pop();
		return element;
	}
private:
	std::queue<T> queue;
	std::mutex mutex;
	std::condition_variable enqueueNotifier;
};

}

#endif
