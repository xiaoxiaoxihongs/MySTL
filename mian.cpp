#include <iostream>
#include "vector.h"
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


int main()
{
	
	MySTL::vector<x> arr_x(5, 5);
	// std::vector<x> arr_x{ 1, 2, 3, 4, 5, };
	std::cout << counter<x>::created << std::endl;
	std::cout << counter<x>::alive << std::endl;
	for (auto& i : arr_x)
		i.print();
	counter<x> arr(arr_x[0]);
	arr.iterface();
	
}