#ifndef SQUALL_PARTIAL_APPLY_HPP_
#define SQUALL_PARTIAL_APPLY_HPP_

#include <utility>

namespace squall {

namespace detail {

template <class F, class Arg>
struct partial_application {
    F   f;
    Arg arg;

    partial_application(F&& f, Arg&& arg)
        : f(std::forward<F>(f)), arg(std::forward<Arg>(arg)) {
    }

    /* 
     * The return type of F only gets deduced based on the number of arguments
     * supplied. PartialApplication otherwise has no idea whether f takes 1 or 10 args.
     */
    template <class... Args>
    auto operator() (Args&& ...args) const
        -> decltype(f(arg, std::declval<Args>()...)) {
        return f(arg, std::forward<Args>(args)...);
    }
};

}

template <class F, class A>
detail::partial_application<F,A> partial(F&& f, A&& a) {
    return detail::partial_application<F, A>(
        std::forward<F>(f), std::forward<A>(a));
}

/* Recursively apply for multiple arguments. */
template <class F, class A, class B>
auto partial(F&& f, A&& a, B&& b)
    -> decltype(partial(partial(std::declval<F>(), std::declval<A>()),
                        std::declval<B>())) {
    return partial(
        partial(std::forward<F>(f), std::forward<A>(a)), std::forward<B>(b));
}

/* Allow n-ary application. */
template<class F, class A, class B, class... C>
auto partial(F&& f, A&& a, B&& b, C&& ...c)
    -> decltype(partial(partial(std::declval<F>(), std::declval<A>()),
                        std::declval<B>(), std::declval<C>()...)) {
    return partial(partial(std::forward<F>(f), std::forward<A>(a)),
                   std::forward<B>(b), std::forward<C>(c)...);
}


}

#endif // SQUALL_PARTIAL_APPLY_HPP_
