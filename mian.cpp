#include <iostream>
#include "MySTL_headers/vector.h"
#include<vector>

template<class T>
struct counter
{
	static int created;
	static int alive;

	counter() { ++created; ++alive; }
	counter(const T&){ ++created; ++alive; }

	void iterface() { 
		std::cout << "print interface" << std::endl;
		static_cast<T*>(this)->print(); 
	}

//protected:
	~counter() { --alive; }
};

template<class T> int counter<T>::created(0);
template<class T> int counter<T>::alive(0);

class x :public counter<x>
{
public:
	x() { std::cout << "x create" << std::endl; }
	x(const int& ){ std::cout << "x create" << std::endl; }
	x(const x& v){ std::cout << "x copy" << std::endl; }

	~x(){ std::cout << "x free" << std::endl; }
	void print() { std::cout << "x " << std::endl; }
};

class y :counter<y>
{
public:
	y() { std::cout << "y create"; }
};

class z :counter<z>
{
public:
	z() { std::cout << "z create"; }
};


template<class T>
void foo(T&)
{
	std::cout << "template<class T> foo" << std::endl;
}

template<>
void foo<int>(int&)
{
	std::cout << "template<> foo" << std::endl;
}


void foo(int&)
{
	std::cout << "int foo" << std::endl;
}

template<class T>
int foo(const T& t)
{
	return t;
}

template<class T, class... Tf>
int foo(const T& t, const Tf& ... res)
{
	return  t + foo(res...);
}

template<class T>
struct NumericalFunctions
{
	T& underlying() { return static_cast<T&>(*this); }
	T const& underlying() const { return static_cast<T const&>(*this);}
	void scale(double multiplicator)
	{
		this->underlying.setValue(this->underlying.getValue() * multiplicator);
	}
	void square()
	{
		T& underlying = static_cast<T&>(*this);
		double temp = underlying.getValue();
		underlying.setValue(temp * temp);
	}
	void setToOpposite();
};

class Sensitivity : public NumericalFunctions<Sensitivity>
{
public:
	Sensitivity() : _value(0) {}
	double getValue() const
	{
		return _value;
	}
	void setValue(double value)
	{
		_value = value;
	}
private:
	double _value;
};


class Foo {
public:
	Foo() = default;
	Foo(const int value) :x(value) { std::cout << "value construct and x= " << x << std::endl; }
	Foo(const Foo& copy) :x(copy.x) { std::cout << "copy construct and x= " << x  << std::endl; }
	Foo& operator=(Foo&& m) noexcept { std::swap(x, m.x); }
	Foo(Foo&& T) noexcept : x(T.x) { std::cout << "move construct and x= " << x << std::endl; T.x = 0; }
	
	~Foo(){ std::cout << "desstruct" << std::endl; }
private:
	int x = 0;
};

template<typename T>
void bar(T&& x) {
	Foo f(std::forward<T>(x));
}

Foo create()
{
	Foo f(10);
	return f;
}

int main()
{
	
	std::cout << "f1(1)" << std::endl;
	Foo f1(1);
	std::cout << std::endl;

	std::cout << "f2(f1)" << std::endl;
	Foo f2(f1);
	std::cout << std::endl;
	

	std::cout << "bar(f1)" << std::endl;
	bar(f1);
	std::cout << std::endl;

	
	std::cout << "bar(Foo(2))" << std::endl;
	bar(Foo(2));
	std::cout << std::endl;

	std::cout << "bar(create)" << std::endl;
	bar(create());
	std::cout << std::endl;

	std::cout << "=" << std::endl;
	Foo f4 = create();
	std::cout << std::endl;

	return 0;
}