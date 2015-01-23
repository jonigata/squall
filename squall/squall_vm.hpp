#ifndef SQUALL_VM_HPP_
#define SQUALL_VM_HPP_

#include <cassert>
#include <functional>
#include <cstdint>
#include "squall_klass_table.hpp"
#include "squall_exception.hpp"
#include "squall_stack_operation.hpp"
#include "squall_call.hpp"
#include "squall_defun.hpp"
#include "squall_make_function.hpp"
#include "squall_table_base.hpp"
#include "squall_closure.hpp"

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
        HSQUIRRELVM vm = handle();
        sq_setforeignptr(vm, &klass_table_);

        // root table取得
        sq_pushroottable(vm);
        sq_getstackobj(vm, -1, &root_);
        sq_pop(vm, -1);

        root_table_.reset(new TableBase(vm, root_));
    }
    ~VM() { sq_setforeignptr(handle(), 0); }

    template <class R, class... T>

    R call(const string& name, T... args) {
        return root_table_->call<R>(name, args...);
    }

    template <class F>
    void defun(const string& name, F f) {
        root_table_->defun(name, f);
    }

    void defraw(const string& s, SQInteger (*f)(HSQUIRRELVM)) {
        root_table_->defraw(s, f);
    }

    template <class... T>
    Coroutine co_call(const string& name, T... args) {
        return root_table_->co_call(name, args...);
    }

    TableBase& root_table() { return *root_table_.get(); }
    KlassTable& klass_table() { return klass_table_; }
    HSQUIRRELVM handle() { return imp_.handle(); }

private:
    detail::VMImp               imp_;
    HSQOBJECT                   root_;
    KlassTable                  klass_table_;
    std::unique_ptr<TableBase>  root_table_;
    
};

}

#endif // SQUALL_VM_HPP

