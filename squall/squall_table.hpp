#ifndef SQUALL_TABLE_HPP_
#define SQUALL_TABLE_HPP_

#include <squirrel.h>
#include "squall_stack_operation.hpp"

namespace squall {

class Table {
public:
    Table(VM& vm) : vm_(vm.handle()) {
        sq_newtable(vm_);
        sq_getstackobj(vm_, -1, &tableobj_);
        sq_addref(vm_, &tableobj_);
    }
    ~Table() {
        sq_release(vm_, &tableobj_);
    }

    template <class T>
    void set(const std::string& name, const T& v) {
        sq_pushobject(vm_, tableobj_);
        sq_pushstring(vm_, name.data(), name.length());
        detail::push(vm_, v);
        sq_newslot(vm_, -3, SQFalse);
        sq_pop(vm_, 1);
    }

    template <class T>
    T get(const std::string& name) {
        T r;
        if(get<T>(name, r)) {
            return r;
        }
        throw squirrel_error("slot '" + name + "' not found");
    }

    template <class T>
    bool get(const std::string& name, T& r) {
        sq_pushobject(vm_, tableobj_);
        sq_pushstring(vm_, name.data(), name.length());
        if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
            return false;
        }
        r = detail::fetch<T>(vm_, -1);
        sq_pop(vm_, 2);
        return true;
    }

    template <class R, class... T>
    R call(const std::string& name, T... args) {
        return detail::call<R>(vm_, tableobj_, name, args...);
    }

    template <class F>
    void defun(const std::string& name, F f) {
        detail::defun_global(vm_, tableobj_, name, to_function(f));
    }

    void defraw(const std::string& s, SQInteger (*f)(HSQUIRRELVM)) {
        detail::defraw(vm_, tableobj_, s, f);
    }

private:
    HSQUIRRELVM vm_;
    HSQOBJECT   tableobj_;

};

}

#endif // SQUALL_TABLE_HPP_
