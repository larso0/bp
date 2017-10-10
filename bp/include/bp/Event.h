#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <functional>

namespace bp
{

/*
 * Event representation for a simple event-delegate system.
 * Delegates are std::function objects.
 * Use the connect functions below, or the attach method to attach delegates.
 */
template<typename... ParamTypes>
class Event
{
public:
	Event() {}

	Event(Event&& other)
	{
		delegates = std::move(other.delegates);
	}

	Event& operator=(Event&& other)
	{
		delegates = std::move(other.delegates);
	}

	void operator()(ParamTypes... args)
	{
		for (std::function<void(ParamTypes...)>& d : delegates)
			d(args...);
	}

	void attach(std::function<void(ParamTypes...)> d)
	{
		delegates.push_back(d);
	}

private:
	std::vector<std::function<void(ParamTypes...)>> delegates;
};

/*
 * Attach a functor delegate.
 * The functor could be a function or a closure.
 */
template<typename... ParamTypes, typename Functor>
void connect(Event<ParamTypes...>& e, Functor f)
{
	e.attach(f);
}

/*
 * Attach a method delegate for the object o.
 */
template<class T, typename... ParamTypes>
void connect(Event<ParamTypes...>& e, T& o, void(T::*m)(ParamTypes...))
{
	e.attach([&o, m](ParamTypes... args) { (o.*m)(args...); });
}

}

#endif