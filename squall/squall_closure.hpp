#ifndef SQUALL_CLOSURE_HPP_
#define SQUALL_CLOSURE_HPP_

#include "squall_closure_decl.hpp"

namespace squall {

namespace detail {

template <class R, class... A>
R Closure<R (A...)>::operator()(A... args) {
    HSQUIRRELVM vm = imp_->vm_;
    keeper k(vm);
    sq_pushobject(vm, imp_->closure_);
    sq_pushnull(vm);
    detail::call_setup_arg(vm, args...);
    if (!SQ_SUCCEEDED(sq_call(vm, sizeof...(args)+1, SQTrue, SQTrue))) {
        throw squirrel_error("closure call failed");
    }
    return detail::call_teardown<R>(vm);
}

}

}

#endif // SQUALL_CLOSURE_HPP_
