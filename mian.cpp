#include <iostream>
#include "MySTL_headers/stack.h"
#include<string>]
#include"MySTL_headers/vector.h"


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

void fun1(int a, int n)
{
	int sum = 0;
	int temp = a;
	for (; n > 0; --n)
	{
		sum += temp;
		temp = temp * 10 + a;
	}
	std::cout << "sum:" << sum << std::endl;
}

int num(vector<int>&num,int target) {

	for (int i = 0; i < num.size() / 2; i++) {
		for (int j=0; j < i; j++) {
			if (num[i] + num[j] == target)
				return i, j;
		}
	}
}

void test1()
{
	std::cout << "输入a， n的值：";
	int a = 0, n = 0;
	std::cin >> a >> n;
	fun1(a, n);
}

int fun2(int n)
{
	if (n == 0) return 0;
	if (n == 1) return 1;
	return fun2(n - 1) + fun2(n - 2);
}


void test2()
{
	int n = 0;
	std::cout << "输入n的值：";
	std::cin >> n;
	int sum = 0;
	for (int i = 0; i < n; ++i)
	{
		int temp = fun2(i);
		std::cout  << temp << " ";
		sum += temp;
	}
	std::cout << "sum:" << sum;
}

int main()
{
	//deque<int> dq(2, 3);
	//prd(dq);
	//std::cout << dq.cfront() << std::endl;
	//std::cout << dq.cback() << std::endl;
	//stack<int> sk;
	//sk.push(1);
	//std::cout << sk.ctop();

	//stack<int> sk2;
	//sk.push(2);
	//sk2.push(3);
	//std::cout << sk.size();
	
	// prs(sk);
	test1();
	test2();
	int num[5];

	
	return 0;
}