#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <squirrel.h>
#include <string>
#include <codecvt>
#include <locale>
#include <type_traits>

namespace squall {

using string = std::basic_string<SQChar>;

namespace detail {

template <class T>
struct _locale_converter {
};

template <>
struct _locale_converter<char> {
    static const std::string& to_std_string(const std::string& s) {
        return s;
    }
    static const std::string& to_squall_string(const std::string& s) {
        return s;
    }
};

template <>
struct _locale_converter<wchar_t> {
    static std::string to_std_string(const std::wstring& s) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(s);
    }
    static std::wstring to_squall_string(const std::string& s) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(s);
    }
};

}

using locale_converter = detail::_locale_converter<SQChar>;

////////////////////////////////////////////////////////////////
// stack keeper
struct keeper {
    keeper(HSQUIRRELVM v) { vm = v; top = sq_gettop(vm); }
    ~keeper() { sq_settop(vm, top); }
    HSQUIRRELVM vm;
    SQInteger top;
};

////////////////////////////////////////////////////////////////
// type wrapper
struct string_wrapper {
    string_wrapper(const SQChar* p) : s_(p) {}
    string_wrapper(const string& s) : s_(s.c_str()) {}
    const SQChar* s_;

    operator const SQChar*() const { return s_; }
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
template <> struct wrapped_type<string> {
    typedef string_wrapper  wrapper_type;
    typedef string          value_type;
};
template <> struct wrapped_type<const string> {
    typedef string_wrapper  wrapper_type;
    typedef string          value_type;
};
template <> struct wrapped_type<string&> {
    typedef string_wrapper  wrapper_type;
    typedef string          value_type;
};
template <> struct wrapped_type<const string&> {
    typedef string_wrapper  wrapper_type;
    typedef string          value_type;
};

template <class T>
typename wrapped_type<T>::wrapper_type
wrap_type(T x) { return typename wrapped_type<T>::wrapper_type(x); }

template <class T> struct unwrapped_type {
    typedef T value_type;
};
template <class T> struct unwrapped_type<T*> {
    typedef T* value_type;
};
template <> struct unwrapped_type<string_wrapper> {
    typedef const SQChar* value_type;
};

template <class T>
typename unwrapped_type<T>::value_type
unwrap_type(T x) { return typename unwrapped_type<T>::value_type(x); }

inline
string get_type_text(SQObjectType t) {
    switch(t) {
        case OT_NULL:           return _SC("null");
        case OT_INTEGER:        return _SC("integer");
        case OT_FLOAT:          return _SC("float");
        case OT_STRING:         return _SC("string");
        case OT_TABLE:          return _SC("table");
        case OT_ARRAY:          return _SC("array");
        case OT_USERDATA:       return _SC("userdata");
        case OT_CLOSURE:        return _SC("closurefunction");
        case OT_NATIVECLOSURE:  return _SC("native closureC function");
        case OT_GENERATOR:      return _SC("generator");
        case OT_USERPOINTER:    return _SC("userpointer");
        case OT_CLASS:          return _SC("class");
        case OT_INSTANCE:       return _SC("instance");
        case OT_WEAKREF:        return _SC("weak reference");
        default:                return _SC("unknown");
    }
}

template <class S>
void print_stack_object(S& s, HSQUIRRELVM vm, SQInteger idx) {
    SQObjectType t = sq_gettype(vm, idx);
    s << get_type_text(t);
}
		

}

#endif // UTILITY_HPP_

