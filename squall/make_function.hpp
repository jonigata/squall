#ifndef MAKE_FUNCTION_HPP_
#define MAKE_FUNCTION_HPP_

#include <functional>

namespace squall {

namespace detail {

template <typename F>
struct callable_traits;

template <class T, class C, class... A>
struct callable_traits<T (C::*)(A...)> {
    using type = T(A...);
};

template <class T, class C, class... A>
struct callable_traits<T (C::*)(A...) const> {
    using type = T(A...);
};
 
template <typename F>
struct function_traits
    : public callable_traits<decltype(&F::operator())> {
};

template <class T, class C, class... A>
struct function_traits<T (C::*)(A...)> {
    using type = T(C*, A...);
};

template <class T, class C, class... A>
struct function_traits<T (C::*)(A...) const> {
    using type = T(const C*, A...);
};
 
template <class T, class... A>
struct function_traits<T (*)(A...)> {
    using type = T (A...);
};

}

template <class F>
std::function<typename detail::function_traits<F>::type>
to_function(F f) {
    return std::function<typename detail::function_traits<F>::type>(f);
}

}

#endif // MAKE_FUNCTION_HPP_
