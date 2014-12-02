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
void call_setup_arg(HSQUIRRELVM vm) {
}

template <class V, class... T> inline
void call_setup_arg(HSQUIRRELVM vm, V head, T... tail) {

    push<V>(vm, head);
    call_setup_arg(vm, tail...);
}

template <class R> inline
R call_teardown(HSQUIRRELVM vm) {
    return fetch<R, detail::FetchContext::ReturnValue>(vm, -1);
}

template <> inline
void call_teardown<void>(HSQUIRRELVM vm) {
}

template <class... T> inline
void call_setup(HSQUIRRELVM vm, const HSQOBJECT& table,
                const string& name, T... args) {

    sq_pushobject(vm, table);
    sq_pushstring(vm, name.data(), name.length());
    if (!SQ_SUCCEEDED(sq_get(vm, -2))) {
        throw squirrel_error("can't find such function: " + name);
    }

    sq_remove(vm, -2);
    sq_pushobject(vm, table);
    call_setup_arg(vm, args...);
    if (!SQ_SUCCEEDED(sq_call(vm, sizeof...(args)+1, SQTrue, SQTrue))) {
        throw squirrel_error("function call failed: " + name);
    }
}

template <class R, class... T> inline
R call(HSQUIRRELVM vm, const HSQOBJECT& table,const string& name, T... args) {

    keeper k(vm);
    call_setup(vm, table, name, args...);
    return call_teardown<R>(vm);
}

template <class... T> inline
void co_call(HSQUIRRELVM vm, const HSQOBJECT& table,
             const string& name, T... args) {

    call_setup(vm, table, name, args...);
}

}

}

#endif // SQUALL_CALL_HPP_
