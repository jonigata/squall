#ifndef SQUALL_CALL_AND_DEFUN_HPP_
#define SQUALL_CALL_AND_DEFUN_HPP_

#include <squirrel.h>
#include "stack_operation.hpp"
#include "partial_apply.hpp"

namespace squall {

namespace detail {

////////////////////////////////////////////////////////////////
// call
inline
int call_setup(HSQUIRRELVM vm, int arity) {
    return arity;
}

template <class V, class... T>
int call_setup(HSQUIRRELVM vm, int arity, V head, T... tail) {
    push<V>(vm, head);
    return call_setup(vm, arity+1, tail...);
}

template <class R>
R call_teardown(HSQUIRRELVM vm) {
    return fetch<R>(vm, -1);
}

template <>
void call_teardown<void>(HSQUIRRELVM vm) {
}

template <class R, class... T>
R call(HSQUIRRELVM vm, const char* name, T... args) {
    keeper k(vm);

    sq_pushroottable(vm);
    sq_pushstring(vm, name, -1);
    if (!SQ_SUCCEEDED(sq_get(vm, -2))) {
        throw squirrel_error(
            std::string("can't find such function: ") + name);
    }

    sq_pushroottable(vm);
    int arity = call_setup(vm, 0, args...);
    if (!SQ_SUCCEEDED(sq_call(vm, arity+1, SQTrue, SQTrue))) {
        throw squirrel_error(std::string("function call failed: ") + name);
    }

    return call_teardown<R>(vm);
}

////////////////////////////////////////////////////////////////
// defun
template <class R>
int stub_aux(HSQUIRRELVM vm, std::function<R ()> f, SQInteger index) {
    push<R>(vm, f());
    return 1;
}

template <>
int stub_aux<void>(HSQUIRRELVM vm, std::function<void ()> f, SQInteger index) {
    f();
    return 0;
}

template <class R, class H, class... T>
int stub_aux(HSQUIRRELVM vm, std::function<R (H, T...)> f, SQInteger index) {
    std::function<R (T...)> newf = partial(f, fetch<H>(vm, index));
    return stub_aux(vm, newf, index + 1);
}

/*
inline
void dp(HSQUIRRELVM vm, int index) {
    SQInteger arg;
    int succeed = SQ_SUCCEEDED(sq_getinteger(vm, index, &arg));
    SQObjectType t = sq_gettype(vm, index);
    printf("stub%d: %lld, succeed = %d, type = %08x\n", index, arg, succeed, t);
}
*/

template <int Offset, class F>
SQInteger stub(HSQUIRRELVM vm) {
    void* fp;
    sq_getuserpointer(vm, -1, &fp);
    // TODO: 例外を拾ってsquirrel例外を出す
    const F& f = *((F*)fp);
    return stub_aux(vm, f, Offset);
}

template <int Offset, class R, class... T>
void defun(
    HSQUIRRELVM vm, const std::string& name, std::function<R (T...)> f) {

    static std::unordered_map<std::string, std::function<R (T...)> > m;
    auto p = &(m[name] = f); // TODO: VMに格納すべき

    sq_pushstring(vm, name.c_str(), -1);
    sq_pushuserpointer(vm, p);
    sq_newclosure(vm, stub<Offset, std::function<R (T...)>>, 1);
    sq_newslot(vm, -3, SQFalse);
}

template <class R, class... T>
void defun_global(
    HSQUIRRELVM vm, const std::string& name, std::function<R (T...)> f) {
    sq_pushroottable(vm);
    defun<2>(vm, name, f);
    sq_pop(vm, 1);
}

template <class R, class... T>
void defun_local(
    HSQUIRRELVM vm, HSQOBJECT table,
    const std::string& name, std::function<R (T...)> f) {
    sq_pushobject(vm, table);
    defun<1>(vm, name, f);
    sq_pop(vm, 1);
}

}

}

#endif // SQUALL_CALL_AND_DEFUN_HPP_
