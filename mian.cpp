#include <iostream>
#include "MySTL_headers/stack.h"
#include<string>

using namespace MySTL;


template<typename T>
void prs(stack<T>& d)
{
	for (const auto& x : d)
		std::cout << x << " ";
	std::cout << std::endl;
}

template<typename T>
void prd(deque<T>& d)
{
	for (const auto& x : d)
		std::cout << x << " ";
	std::cout << std::endl;
}

int main()
{
	deque<int> dq(2, 3);
	prd(dq);
	std::cout << dq.cfront() << std::endl;
	std::cout << dq.cback() << std::endl;
	stack<int> sk;
	sk.push(1);
	std::cout << sk.ctop();

	stack<int> sk2;
	sk.push(2);
	sk2.push(3);
	std::cout << sk.size();
	
	// prs(sk);

	return 0;
}