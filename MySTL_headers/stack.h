#include "deque.h"

namespace MySTL
{
	template<class T, class Sequence = deque<T>>
	class stack;

	template<class T, class Seq>
	bool operator==(const stack<T, Seq>& x, const stack<T, Seq>& y);


	template<class T, class Sequence>
	class stack
	{
	private:
		friend bool operator== <>(const stack&, const stack&);
		// friend bool MySTL::operator< <>(const stack&, const stack&);
		// friend bool operator< (<>(const stack&, const stack&));
	public:
		using value_type		= typename Sequence::value_type;
		using size_type			= typename Sequence::size_type;
		using container_type	= Sequence;
		using reference			= typename Sequence::reference;
		using const_reference	= typename Sequence::const_reference;
	protected:
		Sequence c;
	public:
		stack() :c() {}
		explicit stack(const Sequence& s) : c(s) {}

		template<typename U>
		stack(stack<U>&& s) noexcept :
			c(std::move(s.c))
		{
			s.c->clear();
		}

		void empty() const { c.empty(); }
		size_type size() const { return c.size(); }
		reference top() { return c.back(); }
		const_reference ctop() const { return c.cback(); }
		void push(const value_type& x) { c.push_back(x); }
		void pop() { c.pop_back(); }

	};

	template<class T, class Seq>
	bool operator==(const stack<T, Seq>& x, const stack<T, Seq>& y)
	{
		return x.c == y.c;
	}
}