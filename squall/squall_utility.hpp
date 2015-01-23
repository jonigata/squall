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
        case OT_NULL:           return "null";        
        case OT_INTEGER:        return "integer";
        case OT_FLOAT:          return "float";
        case OT_STRING:         return "string";
        case OT_TABLE:          return "table";
        case OT_ARRAY:          return "array";
        case OT_USERDATA:       return "userdata";
        case OT_CLOSURE:        return "closurefunction";    
        case OT_NATIVECLOSURE:  return "native closureC function";
        case OT_GENERATOR:      return "generator";
        case OT_USERPOINTER:    return "userpointer";
        case OT_CLASS:          return "class";
        case OT_INSTANCE:       return "instance";
        case OT_WEAKREF:        return "weak reference";
        default:                return "unknown";
    }
}

template <class S>
void print_stack_object(S& s, HSQUIRRELVM vm, SQInteger idx) {
    SQObjectType t = sq_gettype(vm, idx);
    s << get_type_text(t);
}
		

}

#endif // UTILITY_HPP_

