#include <iterator>

template< typename C >
class key_iterator
		: public std::iterator<
		  std::bidirectional_iterator_tag,
		  typename C::key_type,
		  typename C::difference_type,
		  typename C::pointer,
		  typename C::reference
		  >
{
public:
	key_iterator() { }
	explicit key_iterator( typename C::const_iterator it ) : it_(it) { }

	const typename C::key_type& operator*() const  { return  it_->first; }
	const typename C::key_type* operator->() const { return &it_->first; }

	key_iterator& operator++() { ++it_; return *this; }
	key_iterator operator++(int) { key_iterator it(*this); ++*this; return it; }

	key_iterator& operator--() { --it_; return *this; }
	key_iterator operator--(int) { key_iterator it(*this); --*this; return it; }

	friend bool operator==( const key_iterator& lhs, const key_iterator& rhs )
	{
		return lhs.it_ == rhs.it_;
	}

	friend bool operator!=( const key_iterator& lhs, const key_iterator& rhs )
	{
		return !(lhs == rhs);
	}

private:
	typename C::const_iterator it_;
};

template <typename C>
key_iterator<C> begin_keys(const C& c) { return key_iterator<C>(c.begin()); }

template <typename C>
key_iterator<C> end_keys(const C& c)   { return key_iterator<C>(c.end());   }
