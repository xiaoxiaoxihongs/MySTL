#include <iostream>

template<class T>
struct counter
{
	static int created;
	static int alive;

	counter() { ++created; ++alive; }
	counter(const T&) { ++created; ++alive; }

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
	x(const int&) { std::cout << "x create" << std::endl; }
	x(const x& v) { std::cout << "x copy" << std::endl; }

	~x() { std::cout << "x free" << std::endl; }
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
	T const& underlying() const { return static_cast<T const&>(*this); }
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
	Foo(const Foo& m_copy) :x(m_copy.x) { std::cout << "copy construct and x= " << x << std::endl; }
	Foo& operator=(Foo&& m) noexcept { std::swap(x, m.x); }
	Foo(Foo&& T) noexcept : x(T.x) { std::cout << "move construct and x= " << x << std::endl; T.x = 0; }

	~Foo() { std::cout << "desstruct" << std::endl; }
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

//template<typename T>
//void prv(const MySTL::vector<T>& vec)
//{
//	for (auto x : vec)
//	{
//		std::cout << x << " ";
//	}
//	std::cout << std::endl;
//}
//
//template<typename T>
//void prv(const std::vector<T>& vec)
//{
//	for (auto x : vec)
//	{
//		std::cout << x << " ";
//	}
//	std::cout << std::endl;
//}
//
//void MySTL_vector_test()
//{
//	MySTL::vector<std::string> mv(2, "hi");
//	prv(mv);
//
//	MySTL::vector<std::string> mv1(mv);
//	prv(mv1);
//
//	MySTL::vector<std::string> mv2(std::move(mv));
//	prv(mv2);
//
//	MySTL::vector<std::string> mv3;
//	mv3 = mv2;
//	prv(mv3);
//
//	MySTL::vector<char> mv4;
//	mv4.push_back('a');
//	mv4[0] = 'b';
//	prv(mv4);
//
//	MySTL::vector<std::string> mv5 = mv1 + mv2;
//	prv(mv5);
//
//	std::cout << mv5.front() << " " << mv5.back() << std::endl;
//
//	for (MySTL::vector<std::string>::iterator it = mv5.begin(); it != mv5.end(); ++it)
//		std::cout << *it << " ** ";
//	std::cout << std::endl;
//	std::cout << mv5.capacity() << std::endl;
//
//	for (int i = 0; i < 5; ++i)
//		mv5.push_back("aa");
//
//	prv(mv5);
//	std::cout << mv5.capacity() << std::endl;
//
//	mv5.insert(mv5.begin() + 4, 15, "bb");
//	prv(mv5);
//
//	std::cout << mv5.capacity() << std::endl;
//
//	mv5.push_back("c");
//	prv(mv5);
//	std::cout << mv5.capacity() << std::endl;
//
//	mv5.pop_back();
//	prv(mv5);
//	std::cout << mv5.size() << " " << mv5.capacity() << std::endl;
//
//	mv5.resize(5);
//	prv(mv5);
//
//	mv2.swap(mv5);
//	prv(mv2);
//}


//template<typename T>
//void prl(MySTL::list<T>& l)
//{
//	for (const auto& x : l)
//		std::cout << x << " ";
//	std::cout << std::endl;
//}
//void MySTL_list_test()
//{
//	MySTL::list<int> li(2, 1);
//	li.push_back(3);
//	prl(li);
//
//	MySTL::list<int> li1;
//	li1 = li;
//	li1.push_front(2);
//	prl(li1);
//
//	MySTL::list<int> li2(li1);
//	li2.pop_front();
//	prl(li2);
//	prl(li1);
//
//	MySTL::list<int> li3(li2);
//	li3.sort();
//	prl(li3);
//
//	for (int i = 0; i < 5; ++i)
//		li3.push_back(i * 2);
//	prl(li3);
//	li3.sort();
//	prl(li3);
//	std::cout << li3.size() << std::endl;
//
//	li3.erase(li3.begin() + 3);
//	prl(li3);
//
//	li3.fill_insert(li3.begin() + 4, 5, 7);
//	prl(li3);
//
//	li3.reverse();
//	prl(li3);
//
//	li3.resize(5);
//	prl(li3);
//
//	MySTL::list<int> li4 = li3;
//	prl(li4);
//
//	MySTL::list<int> li5(std::move(li4));
//	prl(li4);
//	prl(li5);
//
//	if (!li5.empty()) li5.clear();
//	prl(li5);
//
//	li5.merge(li3);
//	prl(li5);
//
//	li5.remove(7);
//	prl(li5);
//}


//template<typename T>
//void prd(deque<T>& d)
//{
//	for (const auto& x : d)
//		std::cout << x << " ";
//	std::cout << std::endl;
//}
//void MySTL_deque_test()
//{
//	deque<char> dq;
//	dq.push_back('a');
//	dq.push_back('v');
//	dq.push_back('c');
//	dq.push_back('g');
//	dq.push_back('a');
//	dq.push_back('v');
//	dq.push_back('c');
//	dq.push_back('g');
//	dq.push_back('a');
//	dq.push_back('v');
//	dq.push_back('c');
//	dq.push_back('g');
//	prd(dq);
//
//	std::cout << dq.size() << std::endl;
//	std::cout << *(dq.begin() + 5) << std::endl;
//	std::cout << (dq.end() - 1) - dq.begin() << std::endl;
//	deque<char>::iterator first = dq.begin() + 1;
//	deque<char>::iterator last = dq.begin() + 4;
//	std::cout << last - first << std::endl;
//
//	dq.erase(dq.begin() + 3, dq.begin() + 6);
//	prd(dq);
//	dq.fill_assign(5, 'o');
//	prd(dq);
//
//	std::cout << dq.size() << std::endl;
//	if (!dq.empty()) dq.resize(24);
//	prd(dq);
//	std::cout << dq.size() << std::endl;
//
//	first = dq.begin() + 1;
//	dq.insert(first, 'j');
//	prd(dq);
//
//	dq.insert(++first);
//	prd(dq);
//
//	dq.resize(4);
//	std::cout << *(first - 1) << std::endl;
//	char arr[3] = { 'a', 's','i' };
//	dq.insert(first, arr, arr + 3);
//	prd(dq);
//	std::cout << dq.size() << std::endl;
//
//	dq.insert(dq.begin() + 1, dq.cbegin(), dq.cbegin() + 2);
//	prd(dq);
//	dq.push_front('t');
//	dq.pop_back();
//	prd(dq);
//
//	std::cout << dq[3] << std::endl;
//}