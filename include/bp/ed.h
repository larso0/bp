#ifndef BP_ED_H
#define BP_ED_H

#include <vector>
#include <functional>

namespace bp
{
	template<typename... ParamTypes> class event
	{
	public:
		void operator()(ParamTypes... args) {
			for (std::function<void(ParamTypes...)>& d : delegates)
				d(args...);
		}

		void attach(std::function<void(ParamTypes...)> d) {
			delegates.push_back(d);
		}

	private:
		std::vector<std::function<void(ParamTypes...)>> delegates;
	};

	template<typename... ParamTypes, typename Functor>
	void connect(event<ParamTypes...>& e, Functor f)
	{
		e.attach(f);
	}

	template<class T, typename... ParamTypes>
	void connect(event<ParamTypes...>& e, T& o, void(T::*m)(ParamTypes...))
	{
		e.attach([&o, m](ParamTypes... args){ (o.*m)(args...); });
	}
}

#endif
