// -*- coding: utf-8-unix -*-

#ifndef SQUALL_STACK_OPERATION_HPP_
#define SQUALL_STACK_OPERATION_HPP_

#include <squirrel.h>
#include "squall_exception.hpp"

namespace squall {

namespace detail {

////////////////////////////////////////////////////////////////
// construct object on stack
template <class T>
void construct_object(HSQUIRRELVM vm, const T& v) {
    SQUserPointer p = sq_newuserdata(vm, sizeof(T*));
    *((T**)p) = new T(v);
    SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
        delete *((T**)p);
        return 0;
    };
    sq_setreleasehook(vm, -1, hook);
}

////////////////////////////////////////////////////////////////
// push
template <class T> inline
void push_aux(HSQUIRRELVM vm, T v) {
    HSQOBJECT sqo;
    if (klass_table(vm).find_klass_object<T>(sqo)) {
        sq_pushobject(vm, sqo);
        sq_createinstance(vm, -1);
        sq_remove(vm, -2);
        sq_setinstanceup(vm, -1, new T(v));
        SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
            delete (T*)p;
            return 0;
        };
        sq_setreleasehook(vm, -1, hook);
    } else {
        SQUserPointer p = sq_newuserdata(vm, sizeof(T));
        new(p) T(v);
        SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
            ((T*)p)->~T();
            return 0;
        };
        sq_setreleasehook(vm, -1, hook);
    }
}
template <class T> inline
void push_aux(HSQUIRRELVM vm, T* v) {
    HSQOBJECT sqo;
    if (klass_table(vm).find_klass_object<T>(sqo)) {
        sq_pushobject(vm, sqo);
        sq_createinstance(vm, -1);
        sq_setinstanceup(vm, -1, v);
        sq_remove(vm, -2);
    } else {
        sq_pushuserpointer(vm, v);
    }
}

template <> inline
void push_aux<int>(HSQUIRRELVM vm, int v) {
    sq_pushinteger(vm, v);
}
template <> inline
void push_aux<float>(HSQUIRRELVM vm, float v) {
    sq_pushfloat(vm, v);
}
template <> inline
void push_aux<bool>(HSQUIRRELVM vm, bool v) {
    sq_pushbool(vm, v ? SQTrue : SQFalse);
}
template <> inline
void push_aux<const char*>(HSQUIRRELVM vm, const char* v) {
    sq_pushstring(vm, v, -1);
}
template <> inline
void push_aux<const std::string&>(HSQUIRRELVM vm, const std::string& v) {
    sq_pushstring(vm, v.data(), v.length());
}

template <class T> inline
void push(HSQUIRRELVM vm, const T& v) {
    push_aux(vm, v);
}

////////////////////////////////////////////////////////////////
// fetch
inline
void check_argument_type(
    HSQUIRRELVM vm, SQInteger index, SQObjectType t, const char* tn) {
    if (sq_gettype(vm, index) != t) {
        throw squirrel_error(
            std::string("value must be ") + tn);
    }
}

template <class T, class F>
T getdata(
    HSQUIRRELVM vm, SQInteger index, SQObjectType t, const char* tn, F f) {
    check_argument_type(vm, index, t, tn);
    T r;
    f(vm, index, &r);
    return r;
}

template <class T>
struct Fetch {
public:
    static T doit(HSQUIRRELVM vm, SQInteger index) {
        check_argument_type(vm, index, OT_USERDATA, "userdata");
        SQUserPointer r;
        sq_getuserdata(vm, index, &r, NULL);
        return **((T**)r);
    }
};

template <class T>
struct Fetch<T*> {
    static T* doit(HSQUIRRELVM vm, SQInteger index) {

        HSQOBJECT sqo;
        if (klass_table(vm).find_klass_object<T>(sqo)) {
            SQUserPointer r;
            sq_getinstanceup(vm, index, &r, NULL);
            return (T*)r;
        } else {
            return (T*)getdata<SQUserPointer>(
                vm, index, OT_USERPOINTER, "userpointer", sq_getuserpointer);
        }
    }
};

template <>
struct Fetch<int> {
    static int doit(HSQUIRRELVM vm, SQInteger index) {
        return getdata<SQInteger>(
            vm, index, OT_INTEGER, "integer", sq_getinteger);
    }
};

template <>
struct Fetch<float> {
    static float doit(HSQUIRRELVM vm, SQInteger index) {
        return getdata<SQFloat>(
            vm, index, OT_FLOAT, "float", sq_getfloat);
    }
};

template <>
struct Fetch<bool> {
    static bool doit(HSQUIRRELVM vm, SQInteger index) {
        return getdata<SQBool>(
            vm, index, OT_BOOL, "bool", sq_getbool);
    }
};

template <>
struct Fetch<std::string> {
    static std::string doit(HSQUIRRELVM vm, SQInteger index) {
        return getdata<const SQChar*>(
            vm, index, OT_STRING, "string", sq_getstring);
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
