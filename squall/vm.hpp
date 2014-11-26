#ifndef VM_HPP_
#define VM_HPP_

#include <squirrel.h>
#include <stdexcept>
#include <cassert>
#include <functional>
#include <unordered_map>
#include "make_function.hpp"
#include "partial_apply.hpp"
//#include "is_dereferencable.hpp"
//#include "any.hpp"

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
void push_aux(HSQUIRRELVM vm, T v) {
    SQUserPointer p = sq_newuserdata(vm, sizeof(T));
    new(p) T(v);
    SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
        ((T*)p)->~T();
        return 1;
    };
    sq_setreleasehook(vm, -1, hook);
}

template <>
void push_aux<int>(HSQUIRRELVM vm, int v) {
    sq_pushinteger(vm, v);
}
template <>
void push_aux<float>(HSQUIRRELVM vm, float v) {
    sq_pushfloat(vm, v);
}
template <>
void push_aux<bool>(HSQUIRRELVM vm, bool v) {
    sq_pushbool(vm, v ? SQTrue : SQFalse);
}
template <>
void push_aux<const char*>(HSQUIRRELVM vm, const char* v) {
    sq_pushstring(vm, v, -1);
}
template <>
void push_aux<const std::string&>(HSQUIRRELVM vm, const std::string& v) {
    sq_pushstring(vm, v.data(), v.length());
}
template <class T>
void push_aux(HSQUIRRELVM vm, T* v) {
    sq_pushuserpointer(vm, v);
}

template <class T> void push(HSQUIRRELVM vm, const T& v) {
    push_aux(vm, v);
}

////////////////////////////////////////////////////////////////
// fetch
inline
void check_argument_type(
    HSQUIRRELVM vm, SQInteger index, SQObjectType t, const char* tn) {
    if (sq_gettype(vm, index) != t) {
        printf("!!!! %08x\n", sq_gettype(vm, index));
        throw squirrel_error(
            std::string("return value must be ") + tn);
    }
}

template <class T>
struct Fetch {
public:
    static T doit(HSQUIRRELVM vm, SQInteger index) {
        check_argument_type(vm, index, OT_USERDATA, "userdata");
        SQUserPointer r;
        sq_getuserdata(vm, index, &r, NULL);
        return *((const T*)r);
    }
};

template <class T>
struct Fetch<T*> {
    static T* doit(HSQUIRRELVM vm, SQInteger index) {
        check_argument_type(vm, index, OT_USERPOINTER, "userpointer");
        SQUserPointer r;
        sq_getuserpointer(vm, index, &r);
        return (T*)r;
    }
};

template <>
struct Fetch<int> {
    static int doit(HSQUIRRELVM vm, SQInteger index) {
        check_argument_type(vm, index, OT_INTEGER, "integer");
        SQInteger r;
        sq_getinteger(vm, index, &r);
        return r;
    }
};

template <>
struct Fetch<float> {
    static float doit(HSQUIRRELVM vm, SQInteger index) {
        check_argument_type(vm, index, OT_FLOAT, "float");
        SQFloat r;
        sq_getfloat(vm, index, &r);
        return r;
    }
};

template <>
struct Fetch<bool> {
    static bool doit(HSQUIRRELVM vm, SQInteger index) {
        check_argument_type(vm, index, OT_BOOL, "bool");
        SQBool r;
        sq_getbool(vm, index, &r);
        return r;
    }
};

template <>
struct Fetch<const char*> {
    static const char* doit(HSQUIRRELVM vm, SQInteger index) {
        check_argument_type(vm, index, OT_STRING, "string");
        const SQChar* r;
        sq_getstring(vm, index, &r);
        return r;
    }
};

template <>
struct Fetch<std::string> {
    static std::string doit(HSQUIRRELVM vm, SQInteger index) {
        check_argument_type(vm, index, OT_STRING, "string");
        const SQChar* r;
        sq_getstring(vm, index, &r);
        return std::string(r);
    }
};

template <class T>
T fetch(HSQUIRRELVM vm, SQInteger index) {
    return Fetch<T>::doit(vm, index);
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
