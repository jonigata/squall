#ifndef SQUALL_CALL_HPP_
#define SQUALL_CALL_HPP_

#include <squirrel.h>
#include "squall_stack_operation.hpp"
#include "squall_partial_apply.hpp"

//#include "squall_demangle.hpp"
#include <iostream>

namespace squall {

namespace detail {

////////////////////////////////////////////////////////////////
// call
inline
void call_setup(HSQUIRRELVM vm, int index) {
}

template <class V, class... T> inline
void call_setup(HSQUIRRELVM vm, int index, V head, T... tail) {

    push<V>(vm, head);
    call_setup(vm, index+1, tail...);
}

template <class R> inline
R call_teardown(HSQUIRRELVM vm) {
    return fetch<R>(vm, -1);
}

template <> inline
void call_teardown<void>(HSQUIRRELVM vm) {
}

template <class R, class... T> inline
R call(HSQUIRRELVM vm, const char* name, T... args) {
    keeper k(vm);

    sq_pushroottable(vm);
    sq_pushstring(vm, name, -1);
    if (!SQ_SUCCEEDED(sq_get(vm, -2))) {
        throw squirrel_error(
            std::string("can't find such function: ") + name);
    }

    sq_remove(vm, -2);
    sq_pushroottable(vm);
    call_setup(vm, 0, args...);
    if (!SQ_SUCCEEDED(sq_call(vm, sizeof...(args)+1, SQTrue, SQTrue))) {
        throw squirrel_error(std::string("function call failed: ") + name);
    }

    return call_teardown<R>(vm);
}

}

}

#endif // SQUALL_CALL_HPP_
