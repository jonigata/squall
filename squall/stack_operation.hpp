#ifndef SQUALL_STACK_OPERATION_HPP_
#define SQUALL_STACK_OPERATION_HPP_

#include <squirrel.h>
#include "exception.hpp"

namespace squall {

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
        throw squirrel_error(
            std::string("return value must be ") + tn);
    }
}

template <class T>
struct Fetch {
public:
    static const T& doit(HSQUIRRELVM vm, SQInteger index) {
        check_argument_type(vm, index, OT_USERDATA, "userdata");
        SQUserPointer r;
        sq_getuserdata(vm, index, &r, NULL);
        // TODO: 型チェック
        return *((T*)r);
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

template <>
struct Fetch<const std::string> : public Fetch<std::string> {
};

template <class T>
typename std::remove_reference<T>::type
 fetch(HSQUIRRELVM vm, SQInteger index) {
    return Fetch<typename std::remove_reference<T>::type>::doit(vm, index);
}

}

}

#endif // SQUALL_STACK_OPERATION_HPP_
