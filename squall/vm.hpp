#ifndef SQUALL_VM_HPP_
#define SQUALL_VM_HPP_

#include <squirrel.h>
#include <cassert>
#include <functional>
#include <unordered_map>
#include <memory>
#include "exception.hpp"
#include "stack_operation.hpp"
#include "call_and_defun.hpp"
#include "make_function.hpp"
//#include "is_dereferencable.hpp"
//#include "any.hpp"

namespace squall {

namespace detail {

////////////////////////////////////////////////////////////////
// klass base
class KlassImpBase {
public:
    virtual ~KlassImpBase() {}
};

typedef std::shared_ptr<KlassImpBase> klass_ptr;

}

////////////////////////////////////////////////////////////////
// VM interface
class VM {
public:
    VM(int stack_size = 1024) {
        vm_ = sq_open(stack_size);
    }
    ~VM() { sq_close(vm_); }

    template <class R, class... T>
    R call(const char* name, T... args) {
        return detail::call<R>(vm_, name, args...);
    }

    template <class R, class... T>
    R call(const std::string& name, T... args) {
        return detail::call<R>(vm_, name.c_str(), args...);
    }

    template <class F>
    void defun(const char* name, F f) {
        detail::defun_global(vm_, name, to_function(f));
    }

    template <class F>
    void defun(const std::string& name, F f) {
        defun(name.c_str(), f);
    }

    void printtop(const char* s) {
        printf("%s: %lld\n", s, sq_gettop(vm_));
    }

    template <class K>
    std::shared_ptr<K> add_klass(const std::string& name) {
        auto i = klasses_.find(name);
        if (i == klasses_.end()) {
            auto p = std::make_shared<K>(handle(), name.c_str());
            klasses_[name] = p;
            return p;
        } else {
            return std::dynamic_pointer_cast<K>((*i).second);
        }
    }

    HSQUIRRELVM handle() { return vm_; }

private:
    HSQUIRRELVM vm_;
    std::unordered_map<std::string, detail::klass_ptr> klasses_;
    
};

}

#endif // SQUALL_VM_HPP

