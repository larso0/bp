#ifndef BP_POINTER_H
#define BP_POINTER_H

#include <stdexcept>
#include <functional>

namespace bp
{

template <typename T>
class NotNull
{
public:
	NotNull(T* t) : t{t}
	{
		if (t == nullptr)
			throw std::invalid_argument("NotNull recieved nullptr.");
	}

	operator T*() { return t;}
	operator const T*() const { return t; }
	T* operator ->() { return t; }
	const T* operator ->() const { return t; }
	T& operator *() { return *t; }
	const T& operator *() const { return *t; }
	T* get() { return t; }
	const T* get() const { return t; }
	
private:
	T* t;
};

template <typename T>
class Scoped
{
public:
	Scoped() : t{nullptr} {}
	explicit Scoped(T* t, std::function<void(T*)> deleter = [](T* t){ delete t; }) :
		t{t},
		deleter{deleter} {}

	~Scoped()
	{
		if (t != nullptr) deleter(t);
	}

	operator T*() { return t;}
	operator const T*() const { return t; }
	T* operator ->() { return t; }
	const T* operator ->() const { return t; }
	T& operator *() { return *t; }
	const T& operator *() const { return *t; }
	T* get() { return t; }
	const T* get() const { return t; }

private:
	T* t;
	std::function<void(T*)> deleter;
};

}

#endif