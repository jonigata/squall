#ifndef SQUALL_VM_HPP_
#define SQUALL_VM_HPP_

#include <cassert>
#include <functional>
#include "squall_klass_table.hpp"
#include "squall_exception.hpp"
#include "squall_stack_operation.hpp"
#include "squall_call.hpp"
#include "squall_defun.hpp"
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
    VM(int stack_size = 1024) : imp_(stack_size) {
        sq_setforeignptr(handle(), &klass_table_);
    }
    ~VM() { sq_setforeignptr(handle(), 0); }

    template <class R, class... T>
    R call(const std::string& name, T... args) {
        return detail::call<R>(handle(), name, args...);
    }

    template <class F>
    void defun(const std::string& name, F f) {
        detail::defun_global(handle(), name, to_function(f));
    }

    void defraw(const std::string& s, SQInteger (*f)(HSQUIRRELVM)) {
        detail::defraw(handle(), s, f);
    }

    KlassTable& klass_table() { return klass_table_; }
    HSQUIRRELVM handle() { return imp_.handle(); }

private:
    detail::VMImp   imp_;
    KlassTable      klass_table_;
    
};

}

#endif // SQUALL_VM_HPP

