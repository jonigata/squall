#ifndef SQUALL_VM_HPP_
#define SQUALL_VM_HPP_

#include <cassert>
#include <functional>
#include "squall_klass_table.hpp"
#include "squall_exception.hpp"
#include "squall_stack_operation.hpp"
#include "squall_call_and_defun.hpp"
#include "squall_make_function.hpp"

namespace squall {

namespace detail {

class VMImp { // for destruct order
public:
    VMImp(int stack_size = 1024) { vm_ = sq_open(stack_size); }
    ~VMImp() {
        sq_close(vm_);
    }
    HSQUIRRELVM handle() { return vm_; }
private:
    HSQUIRRELVM vm_;
};

}

////////////////////////////////////////////////////////////////
// VM interface
class VM {
public:
    VM(int stack_size = 1024) : imp_(stack_size){}

    template <class R, class... T>
    R call(const char* name, T... args) {
        return detail::call<R>(handle(), klass_table_, name, args...);
    }

    template <class R, class... T>
    R call(const std::string& name, T... args) {
        return detail::call<R>(handle(), klass_table_, name.c_str(), args...);
    }

    template <class F>
    void defun(const char* name, F f) {
        defun(std::string(name), f);
    }

    template <class F>
    void defun(const std::string& name, F f) {
        detail::defun_global(handle(), klass_table_, name, to_function(f));
    }

    KlassTable& klass_table() { return klass_table_; }
    HSQUIRRELVM handle() { return imp_.handle(); }

private:
    detail::VMImp   imp_;
    KlassTable      klass_table_;
    
};

}

#endif // SQUALL_VM_HPP

