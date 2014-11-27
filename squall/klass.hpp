#ifndef SQUALL_KLASS_HPP_
#define SQUALL_KLASS_HPP_

#include <functional>

#include "vm.hpp"
#include "call_and_defun.hpp"
#include "make_function.hpp"

namespace squall {

namespace detail {

template <class K>
class KlassImp : public KlassImpBase {
public:
    KlassImp(HSQUIRRELVM vm, const char* name) {
        vm_ = vm;
        sq_pushroottable(vm); // TODO: root固定になってる
        sq_pushstring(vm, name, -1);
        sq_newclass(vm, SQFalse);
        sq_getstackobj(vm, -1, &sqclass_);
        sq_createslot(vm, -3);
    }

    template <class F>
    void func(const std::string& name, F f) {
        detail::defun_local(vm_, sqclass_, name, to_function(f));
    }

private:
    HSQUIRRELVM vm_;
    HSQOBJECT   sqclass_;
    
};

}

template <class C>
class Klass {
public:
    Klass(const std::string& name, VM& vm) : vm_(vm) {
        imp_ = vm.add_klass<detail::KlassImp<C>>(name);
    }
    Klass(const Klass&) = delete;
    Klass(const Klass&&) = delete;
    void operator=(const Klass&) = delete;
    void operator=(const Klass&&) = delete;
    
    template <class F>
    Klass<C>& func(const char* name, F f) {
        imp_->func(name, f);
        return *this;
    }

    template <class F>
    Klass<C>& func(const std::string& name, F f) {
        imp_->func(name, f);
        return *this;
    }

private:
    VM&                                     vm_;
    std::shared_ptr<detail::KlassImp<C>>    imp_;

};

}

#endif // SQUALL_KLASS_HPP_
