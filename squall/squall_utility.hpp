#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <squirrel.h>
#include <string>

namespace squall {

using string = std::basic_string<SQChar>;

////////////////////////////////////////////////////////////////
// stack keeper
struct keeper {
    keeper(HSQUIRRELVM v) { vm = v; top = sq_gettop(vm); }
    ~keeper() { sq_settop(vm, top); }
    HSQUIRRELVM vm;
    int top;
};

////////////////////////////////////////////////////////////////
// type wrapper
struct string_wrapper {
    string_wrapper(const SQChar* p) : p_(p) {}
    string_wrapper(const string& s) : p_(s.c_str()) {}
    const SQChar* p_;

    operator const SQChar*() const { return p_; }
};

template <class T> struct wrapped_type {
    typedef T wrapper_type;
    typedef T value_type;
};
template <class T> struct wrapped_type<T*> {
    typedef T* wrapper_type;
    typedef T* value_type;
};
template <> struct wrapped_type<const SQChar*> {
    typedef string_wrapper  wrapper_type;
    typedef string          value_type;
};
template <> struct wrapped_type<std::string> {
    typedef string_wrapper  wrapper_type;
    typedef string          value_type;
};
template <> struct wrapped_type<const std::string> {
    typedef string_wrapper  wrapper_type;
    typedef string          value_type;
};
template <> struct wrapped_type<std::string&> {
    typedef string_wrapper  wrapper_type;
    typedef string          value_type;
};
template <> struct wrapped_type<const std::string&> {
    typedef string_wrapper  wrapper_type;
    typedef string          value_type;
};

template <class T>
typename wrapped_type<T>::wrapper_type
wrap_type(T x) { return typename wrapped_type<T>::wrapper_type(x); }

}

#endif // UTILITY_HPP_

