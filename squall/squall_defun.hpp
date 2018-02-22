#ifndef SQUALL_DEFUN_HPP_
#define SQUALL_DEFUN_HPP_

#include <squirrel.h>
#include "squall_stack_operation.hpp"
#include "squall_partial_apply.hpp"
#include "squall_make_function.hpp"

//#include "squall_demangle.hpp"
#include <iostream>

namespace squall {

namespace detail {

////////////////////////////////////////////////////////////////
// defun
template <class F>
struct Stub;

template <>
struct Stub<void ()> {
    template <class F>
    static int doit(HSQUIRRELVM vm, SQInteger index, F f) {
        f();
        return 0;
    }
};

template <class R>
struct Stub<R ()> {
    template <class F>
    static int doit(HSQUIRRELVM vm, SQInteger index, F f) {
        push<R>(vm, f());
        return 1;
    }
};

template <class R, class H, class... T>
struct Stub<R (H, T...)> {
    template <class F>
    static int doit(HSQUIRRELVM vm, SQInteger index, F f) {
        auto newf =
            partial(
                f,
                unwrap_type(
                    fetch<H, detail::FetchContext::Argument>(vm, index)));
        return Stub<R (T...)>::doit(vm, index+1, newf);
    }
};

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

        SQInteger x = Stub<typename detail::function_traits<F>::type>::doit(
            vm, Offset, f);
        return x;
    }
    catch(std::exception& e) {
        return sq_throwerror(
            vm, (string(_SC("error in callback: ")) + locale_converter::to_squall_string(e.what()).c_str()).c_str());
    }
}

template <class T> SQChar typemask() { return _SC('.'); }

template <> inline SQChar typemask<decltype(nullptr)>() { return _SC('o'); }
template <> inline SQChar typemask<int>() { return _SC('i'); }
template <> inline SQChar typemask<float>() { return _SC('f'); }
template <> inline SQChar typemask<bool>() { return _SC('b'); }
template <> inline SQChar typemask<const SQChar*>() { return _SC('s'); }
template <> inline SQChar typemask<string>() { return _SC('s'); }

template <class... T>
struct TypeMaskList;

template <>
struct TypeMaskList<> {
    static string doit() { return _SC(""); }
};

template <class H, class... T>
struct TypeMaskList<H, T...> {
    static string doit() {
        return typemask<H>() + TypeMaskList<T...>::doit();
    }
};

template <int Offset, class R, class... T>
void defun(
    HSQUIRRELVM vm,
    const string& name,
    const std::function<R (T...)>& f,
    const string& argtypemask) {

    sq_pushstring(vm, name.data(), name.length());
    construct_object(vm, f);
    sq_newclosure(vm, stub<Offset, std::function<R (T...)>>, 1);
    sq_setparamscheck(vm, SQ_MATCHTYPEMASKSTRING, argtypemask.c_str());
    sq_setnativeclosurename(vm, -1, name.c_str());
    sq_newslot(vm, -3, SQFalse);
}

template <class R, class... T>
void defun_global(
    HSQUIRRELVM vm, const HSQOBJECT& table,
    const string& name, const std::function<R (T...)>& f) {

    sq_pushobject(vm, table);
    defun<2>(vm, name, f, _SC(".") + TypeMaskList<T...>::doit());
    sq_pop(vm, 1);
}

template <class R, class... T>
void defun_local(
    HSQUIRRELVM vm, const HSQOBJECT& klass_object,
    const string& name, const std::function<R (T...)>& f) {

    sq_pushobject(vm, klass_object);
    defun<1>(vm, name, f, TypeMaskList<T...>::doit());
    sq_pop(vm, 1);
}

inline
void defraw(
    HSQUIRRELVM vm, const HSQOBJECT& table,
    const string& name, SQInteger (*f)(HSQUIRRELVM)) {
    
    //sq_pushroottable(vm);
    sq_pushobject(vm, table);
    sq_pushstring(vm, name.data(), name.length());
    sq_newclosure(vm, f, 0);
    sq_setnativeclosurename(vm, -1, name.c_str());
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 1);
}

template <class R, class... A>
void push_closure(HSQUIRRELVM vm, std::function<R (A...)> v) {
    auto argtypemask = "." + TypeMaskList<A...>::doit();

    construct_object(vm, v);
    sq_newclosure(vm, stub<1, std::function<R (A...)>>, 1);
    sq_setparamscheck(vm, SQ_MATCHTYPEMASKSTRING, argtypemask.c_str());
    sq_setnativeclosurename(vm, -1, "<C++ lambda>");
}

}

}

#endif // SQUALL_DEFUN_HPP_
