#ifndef MAKE_FUNCTION_HPP_
#define MAKE_FUNCTION_HPP_

#include <functional>

namespace squall {

namespace detail {

template <typename Function>
struct function_traits
    : public function_traits<decltype(&Function::operator())> {
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> {
    typedef ReturnType (*pointer)(Args...);
    typedef std::function<ReturnType(Args...)> function;
};

}

template <typename Function>
typename detail::function_traits<Function>::pointer
to_function_pointer (Function& lambda) {
    return static_cast<typename detail::function_traits<Function>::pointer>(
        lambda);
}

template <typename Function>
typename detail::function_traits<Function>::function
to_function (Function& lambda) {
    return static_cast<typename detail::function_traits<Function>::function>(
        lambda);
}

}

#endif // MAKE_FUNCTION_HPP_
