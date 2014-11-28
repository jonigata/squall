#ifndef SQUALL_KLASS_HPP_
#define SQUALL_KLASS_HPP_

#include <functional>

#include "vm.hpp"
#include "call_and_defun.hpp"

namespace squall {

template <class C>
class Klass {
public:
    Klass(VM& vm, const std::string& name) : vm_(vm) {
        imp_ = vm.klass_table().add_klass<C>(vm_.handle(), name);
    }
    ~Klass() { imp_.lock()->close(); }

    Klass(const Klass&) = delete;
    Klass(const Klass&&) = delete;
    void operator=(const Klass&) = delete;
    void operator=(const Klass&&) = delete;
    
    template <class F>
    Klass<C>& func(const char* name, F f) {
        func(std::string(name), f);
        return *this;
    }

    template <class F>
    Klass<C>& func(const std::string& name, F f) {
        detail::defun_local(
            vm_.handle(),
            vm_.klass_table(),
            imp_.lock()->get_klass_object(),
            name,
            to_function(f));
        return *this;
    }

private:
    VM& vm_;
    std::weak_ptr<detail::KlassImp<C>> imp_;

};

}

#endif // SQUALL_KLASS_HPP_
