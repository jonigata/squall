#ifndef MAKE_FUNCTION_HPP_
#define MAKE_FUNCTION_HPP_

#include <functional>

namespace squall {

namespace detail {

template <typename F>
struct functor_traits;

template <class T, class C, class... A>
struct functor_traits<T (C::*)(A...)> {
    typedef T type(A...);
};

template <class T, class C, class... A>
struct functor_traits<T (C::*)(A...) const> {
    typedef T type(A...);
};
 
template <typename F>
struct function_traits
    : public functor_traits<decltype(&F::operator())> {
};

template <class T, class C, class... A>
struct function_traits<T (C::*)(A...)> {
    typedef T type(C*, A...);
};

template <class T, class C, class... A>
struct function_traits<T (C::*)(A...) const> {
    typedef T type(const C*, A...);
};
 
template <class T, class... A>
struct function_traits<T (*)(A...)> {
    typedef T type(A...);
};

}

template <class F>
std::function<typename detail::function_traits<F>::type>
to_function(F f) {
    return std::function<typename detail::function_traits<F>::type>(f);
}

}

#endif // MAKE_FUNCTION_HPP_
