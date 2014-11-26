#ifndef VM_HPP_
#define VM_HPP_

#include <squirrel.h>
#include <stdexcept>
#include <cassert>
#include <functional>
#include <unordered_map>
#include "make_function.hpp"
#include "partial_apply.hpp"

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
template <class T> void push(HSQUIRRELVM vm, T v) {
    assert(0);
}

template <> void push<int>(HSQUIRRELVM vm, int v) {
    sq_pushinteger(vm, v);
}
template <> void push<float>(HSQUIRRELVM vm, float v) {
    sq_pushfloat(vm, v);
}
template <> void push<bool>(HSQUIRRELVM vm, bool v) {
    sq_pushbool(vm, v ? SQTrue : SQFalse);
}
template <> void push<const char*>(HSQUIRRELVM vm, const char* v) {
    sq_pushstring(vm, v, -1);
}
template <> void push<const std::string&>(HSQUIRRELVM vm, const std::string& v) {
    sq_pushstring(vm, v.data(), v.length());
}
template <> void push<void*>(HSQUIRRELVM vm, void* v) {
    sq_pushuserpointer(vm, v);
}

////////////////////////////////////////////////////////////////
// fetch
template <class T> T fetch(HSQUIRRELVM vm, SQInteger index) {
    assert(0);
    return T();
}

inline
void check_argument_type(
    HSQUIRRELVM vm, SQInteger index, SQObjectType t, const char* tn) {
    if (sq_gettype(vm, index) != t) {
        throw squirrel_error(
            std::string("return value must be ") + tn);
    }
}

template <> int fetch<int>(HSQUIRRELVM vm, SQInteger index) {
    check_argument_type(vm, index, OT_INTEGER, "integer");
    SQInteger r;
    sq_getinteger(vm, index, &r);
    return r;
}
template <> float fetch<float>(HSQUIRRELVM vm, SQInteger index) {
    check_argument_type(vm, index, OT_FLOAT, "float");
    SQFloat r;
    sq_getfloat(vm, index, &r);
    return r;
}
template <> bool fetch<bool>(HSQUIRRELVM vm, SQInteger index) {
    check_argument_type(vm, index, OT_BOOL, "bool");
    SQBool r;
    sq_getbool(vm, index, &r);
    return r;
}
template <> const char* fetch<const char*>(HSQUIRRELVM vm, SQInteger index) {
    check_argument_type(vm, index, OT_STRING, "string");
    const SQChar* r;
    sq_getstring(vm, index, &r);
    return r;
}
template <> std::string fetch<std::string>(HSQUIRRELVM vm, SQInteger index) {
    check_argument_type(vm, index, OT_STRING, "string");
    const SQChar* r;
    sq_getstring(vm, index, &r);
    return std::string(r);
}
template <> void* fetch<void*>(HSQUIRRELVM vm, SQInteger index) {
    check_argument_type(vm, index, OT_USERPOINTER, "userpointer");
    SQUserPointer r;
    sq_getuserpointer(vm, index, &r);
    return r;
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
int stub_aux(HSQUIRRELVM vm, std::function<R ()> f, int index) {
    push<R>(vm, f());
    return 1;
}

template <>
int stub_aux<void>(HSQUIRRELVM vm, std::function<void ()> f, int index) {
    f();
    return 0;
}

template <class R, class... T>
int stub_aux(HSQUIRRELVM vm, std::function<R (int, T...)> f, int index) {
    SQInteger arg;
    sq_getinteger(vm, index, &arg);

    std::function<R (T...)> newf = partial(f, arg);
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
