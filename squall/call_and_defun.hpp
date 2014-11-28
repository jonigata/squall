#ifndef SQUALL_CALL_AND_DEFUN_HPP_
#define SQUALL_CALL_AND_DEFUN_HPP_

#include <squirrel.h>
#include "stack_operation.hpp"
#include "partial_apply.hpp"

#include "demangle.hpp"

namespace squall {

namespace detail {

////////////////////////////////////////////////////////////////
// call
inline
int call_setup(HSQUIRRELVM vm, KlassTable&, int arity) {
    return arity;
}

template <class V, class... T> inline
int call_setup(
    HSQUIRRELVM vm, KlassTable& klass_table,
    int arity, V head, T... tail) {

    push<V>(vm, klass_table, head);
    return call_setup(vm, klass_table, arity+1, tail...);
}

template <class R> inline
R call_teardown(HSQUIRRELVM vm, KlassTable& klass_table) {
    return fetch<R>(vm, klass_table, -1);
}

template <> inline
void call_teardown<void>(HSQUIRRELVM vm, KlassTable& klass_table) {
}

template <class R, class... T> inline
R call(HSQUIRRELVM vm, KlassTable& klass_table, const char* name, T... args) {
    keeper k(vm);

    sq_pushroottable(vm);
    sq_pushstring(vm, name, -1);
    if (!SQ_SUCCEEDED(sq_get(vm, -2))) {
        throw squirrel_error(
            std::string("can't find such function: ") + name);
    }

    sq_remove(vm, -2);
    sq_pushroottable(vm);
    int arity = call_setup(vm, klass_table, 0, args...);
    if (!SQ_SUCCEEDED(sq_call(vm, arity+1, SQTrue, SQTrue))) {
        throw squirrel_error(std::string("function call failed: ") + name);
    }

    return call_teardown<R>(vm, klass_table);
}

////////////////////////////////////////////////////////////////
// defun
template <class R> inline
int stub_aux(
    HSQUIRRELVM vm, KlassTable& klass_table,
    const std::function<R ()>& f, SQInteger index) {

    push<R>(vm, klass_table, f());
    return 1;
}

template <> inline
int stub_aux<void>(
    HSQUIRRELVM vm, KlassTable&,
    const std::function<void ()>& f, SQInteger index) {

    f();
    return 0;
}

template <class R, class H, class... T> inline
int stub_aux(
    HSQUIRRELVM vm, KlassTable& klass_table,
    const std::function<R (H, T...)>& f, SQInteger index) {

    std::function<R (T...)> newf =
        partial(f, fetch<H>(vm, klass_table, index));
    return stub_aux(vm, klass_table, newf, index + 1);
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
    try {
        SQUserPointer fp;
        if(!SQ_SUCCEEDED(sq_getuserdata(vm, -1, &fp, NULL))) { assert(0); }
        const F& f = **((F**)fp);

        SQUserPointer cp;
        if(!SQ_SUCCEEDED(sq_getuserpointer(vm, -2, &cp))) { assert(0); }
        KlassTable& klass_table = *((KlassTable*)cp);
    
        //return stub_aux(klass_table, f, Offset);
        SQInteger x = stub_aux(vm, klass_table, f, Offset);
        return x;
    }
    catch(std::exception& e) {
        return sq_throwerror(
            vm, (std::string("error in callback: ") + e.what()).c_str());
    }
}

template <int Offset, class R, class... T>
void defun(
    HSQUIRRELVM vm, KlassTable& klass_table,
    const std::string& name, const std::function<R (T...)>& f) {

    sq_pushstring(vm, name.c_str(), -1);
    construct_object(vm, f);
    sq_pushuserpointer(vm, &klass_table);
    sq_newclosure(vm, stub<Offset, std::function<R (T...)>>, 2);
    //sq_setparamscheck(vm, 0);
    sq_setnativeclosurename(vm, -1, name.c_str());
    sq_newslot(vm, -3, SQFalse);
}

template <class R, class... T>
void defun_global(
    HSQUIRRELVM vm, KlassTable& klass_table,
    const std::string& name, const std::function<R (T...)>& f) {

    sq_pushroottable(vm);
    defun<2>(vm, klass_table, name, f);
    sq_pop(vm, 1);
}

template <class R, class... T>
void defun_local(
    HSQUIRRELVM vm, KlassTable& klass_table, const HSQOBJECT& klass_object,
    const std::string& name, const std::function<R (T...)>& f) {

    sq_pushobject(vm, klass_object);
    defun<1>(vm, klass_table, name, f);
    sq_pop(vm, 1);
}

}

}

#endif // SQUALL_CALL_AND_DEFUN_HPP_
