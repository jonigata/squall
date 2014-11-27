#ifndef SQUALL_IS_DEREFERENCABLE_HPP_
#define SQUALL_IS_DEREFERENCABLE_HPP_

#include <utility>

namespace squall {

namespace detail {

struct dereferencable_tag {};
struct undereferencable_tag {};

template <class>
struct ignore {
    typedef void type;
};

}

template <class T, class X=void>
struct is_dereferencable {
    enum { value = 0 };
    typedef detail::undereferencable_tag tag;
};

template <class T>
struct is_dereferencable<T, typename detail::ignore<decltype(*std::declval<T>())>::type> {
    enum { value = 1 };
    typedef detail::dereferencable_tag tag;
};

}

#endif // SQUALL_IS_DEREFERENCABLE_HPP_
