#ifndef SQUALL_TABLE_BASE_HPP_
#define SQUALL_TABLE_BASE_HPP_

#include <squirrel.h>
#include "squall_stack_operation.hpp"
#include "squall_coroutine.hpp"

namespace squall {

class TableBase {
public:
    TableBase(HSQUIRRELVM vm, const HSQOBJECT& to) : vm_(vm), tableobj_(to) {}

    template <class T>
    void set(const string& name, const T& v) {
        sq_pushobject(vm_, tableobj_);
        sq_pushstring(vm_, name.data(), name.length());
        detail::push(vm_, v);
        sq_newslot(vm_, -3, SQFalse);
        sq_pop(vm_, 1);
    }

    template <class T>
    T get(const string& name) {
        T r;
        if(get<T>(name, r)) {
            return r;
        }
        throw squirrel_error("slot '" + name + "' not found");
    }

    template <class T>
    bool get(const string& name, T& r) {
        sq_pushobject(vm_, tableobj_);
        sq_pushstring(vm_, name.data(), name.length());
        if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
            return false;
        }
        r = detail::fetch<T, detail::FetchContext::TableEntry>(vm_, -1);
        sq_pop(vm_, 2);
        return true;
    }

    template <class R, class... T>
    R call(const string& name, T... args) {
        return detail::call<R>(vm_, tableobj_, name, args...);
    }

    template <class F>
    void defun(const string& name, F f) {
        detail::defun_global(vm_, tableobj_, name, to_function(f));
    }

    void defraw(const string& s, SQInteger (*f)(HSQUIRRELVM)) {
        detail::defraw(vm_, tableobj_, s, f);
    }

    template <class... T>
    Coroutine co_call(const string& name, T... args) {
        SQInteger top = sq_gettop(handle());
        detail::call_setup(vm_, tableobj_, name, args...);
        return Coroutine(vm_, top);
    }

protected:
    HSQUIRRELVM handle() { return vm_; }
    HSQOBJECT&  tableobj() { return tableobj_; }

private:
    HSQUIRRELVM vm_;
    HSQOBJECT   tableobj_;

};

}

#endif // SQUALL_TABLE_BASE_HPP_
