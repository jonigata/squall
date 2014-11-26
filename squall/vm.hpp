#ifndef VM_HPP_
#define VM_HPP_

#include <squirrel.h>
#include <stdexcept>
#include <cassert>
#include <functional>
#include <unordered_map>
#include "make_function.hpp"
#include "partial_apply.hpp"
#include "is_dereferencable.hpp"

namespace squall {

class squirrel_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

namespace detail {

////////////////////////////////////////////////////////////////
// stack keeper
struct keeper {
    keeper(HSQUIRRELVM v) { vm = v; top = sq_gettop(vm); }
    ~keeper() { sq_settop(vm, top); }
    HSQUIRRELVM vm;
    int top;
};

////////////////////////////////////////////////////////////////
// push
template <class T>
void push_aux(HSQUIRRELVM vm, T v, undereferencable_tag);

template <>
void push_aux<int>(HSQUIRRELVM vm, int v, undereferencable_tag) {
    sq_pushinteger(vm, v);
}
template <>
void push_aux<float>(HSQUIRRELVM vm, float v, undereferencable_tag) {
    sq_pushfloat(vm, v);
}
template <>
void push_aux<bool>(HSQUIRRELVM vm, bool v, undereferencable_tag) {
    sq_pushbool(vm, v ? SQTrue : SQFalse);
}
template <>
void push_aux<const char*>(HSQUIRRELVM vm, const char* v, undereferencable_tag) {
    sq_pushstring(vm, v, -1);
}
template <>
void push_aux<const std::string&>(HSQUIRRELVM vm, const std::string& v, undereferencable_tag) {
    sq_pushstring(vm, v.data(), v.length());
}
template <>
void push_aux<void*>(HSQUIRRELVM vm, void* v, undereferencable_tag) {
    sq_pushuserpointer(vm, v);
}

template <class T>
void push_aux(HSQUIRRELVM vm, T v, dereferencable_tag) {
    sq_pushuserpointer(vm, &*v);
}

template <class T> void push(HSQUIRRELVM vm, T v) {
    push_aux(vm, v, typename is_dereferencable<T>::tag());
}

////////////////////////////////////////////////////////////////
// fetch
inline
void check_argument_type(
    HSQUIRRELVM vm, SQInteger index, SQObjectType t, const char* tn) {
    if (sq_gettype(vm, index) != t) {
        throw squirrel_error(
            std::string("return value must be ") + tn);
    }
}

template <class T>
T fetch_aux(HSQUIRRELVM vm, SQInteger index, undereferencable_tag);

template <>
int fetch_aux<int>(HSQUIRRELVM vm, SQInteger index, undereferencable_tag) {
    check_argument_type(vm, index, OT_INTEGER, "integer");
    SQInteger r;
    sq_getinteger(vm, index, &r);
    return r;
}
template <>
float fetch_aux<float>(HSQUIRRELVM vm, SQInteger index, undereferencable_tag) {
    check_argument_type(vm, index, OT_FLOAT, "float");
    SQFloat r;
    sq_getfloat(vm, index, &r);
    return r;
}
template <>
bool fetch_aux<bool>(HSQUIRRELVM vm, SQInteger index, undereferencable_tag) {
    check_argument_type(vm, index, OT_BOOL, "bool");
    SQBool r;
    sq_getbool(vm, index, &r);
    return r;
}
template <>
const char* fetch_aux<const char*>(
    HSQUIRRELVM vm, SQInteger index, undereferencable_tag) {
    check_argument_type(vm, index, OT_STRING, "string");
    const SQChar* r;
    sq_getstring(vm, index, &r);
    return r;
}
template <>
std::string fetch_aux<std::string>(
    HSQUIRRELVM vm, SQInteger index, undereferencable_tag) {
    check_argument_type(vm, index, OT_STRING, "string");
    const SQChar* r;
    sq_getstring(vm, index, &r);
    return std::string(r);
}
template <>
void* fetch_aux<void*>(HSQUIRRELVM vm, SQInteger index, undereferencable_tag) {
    check_argument_type(vm, index, OT_USERPOINTER, "userpointer");
    SQUserPointer r;
    sq_getuserpointer(vm, index, &r);
    return r;
}

template <class T>
T fetch_aux(HSQUIRRELVM vm, SQInteger index, dereferencable_tag) {
    check_argument_type(vm, index, OT_USERPOINTER, "userpointer");
    SQUserPointer r;
    sq_getuserpointer(vm, index, &r);
    return static_cast<T>(r); // TODO: support smart pointer
}

template <class T>
T fetch(HSQUIRRELVM vm, SQInteger index) {
    return fetch_aux<T>(vm, index, typename is_dereferencable<T>::tag());
}

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
    return fetch<int>(vm, -1);
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

template <class F>
SQInteger stub(HSQUIRRELVM vm) {
    void* fp;
    sq_getuserpointer(vm, -1, &fp);
    const F& f = *((F*)fp);
    return stub_aux(vm, f, 2);
}

template <class R, class... T>
void defun(
    HSQUIRRELVM vm, const std::string& name, std::function<R (T...)> f) {
    static std::unordered_map<std::string, std::function<R (T...)> > m;
    auto p = &(m[name] = f); // TODO: VMごとにする
    sq_pushroottable(vm);
    sq_pushstring(vm, name.c_str(), -1);
    sq_pushuserpointer(vm, p);
    sq_newclosure(vm, stub<std::function<R (T...)> >, 1);
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 1);
}

}

////////////////////////////////////////////////////////////////
// interface class
class VM {
public:
    VM(int stack_size = 1024) {
        vm_ = sq_open(stack_size);
    }
    ~VM() { sq_close(vm_); }

    template <class R, class... T>
    R call(const char* name, T... args) {
        return detail::call<R>(vm_, name, args...);
    }

    template <class R, class... T>
    R call(const std::string& name, T... args) {
        return detail::call<R>(vm_, name.c_str(), args...);
    }

    template <class F>
    void defun(const char* name, F f) {
        detail::defun(vm_, name, to_function(f));
    }

    template <class F>
    void defun(const std::string& name, F f) {
        detail::defun(vm_, name, to_function(f));
    }

    void printtop(const char* s) {
        printf("%s: %lld\n", s, sq_gettop(vm_));
    }

private:
protected:
    HSQUIRRELVM handle() { return vm_; }

private:
    HSQUIRRELVM vm_;
};

}

#endif // VM_HPP_
